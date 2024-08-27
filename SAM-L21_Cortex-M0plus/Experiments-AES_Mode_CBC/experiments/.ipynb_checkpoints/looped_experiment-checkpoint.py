

from os import path
import pickle
import json
from lmfit import Model

from pydgilib_extra import (
    DGILibExtra, power_and_time_per_pulse, LoggerData, LOGGER_OBJECT,
    LOGGER_CSV)
from atprogram.atprogram import atprogram


def looped_experiment(config_file=path.abspath(
        path.join(path.curdir, "looped_experiment.json")), device_name = "ATSAML21J18B",
        verbose=2, config_dict={}, live_plot=False, max_log_time=1000,
        log_stop_function=None, analysis_stop_function=None,
        drop_last_samples=0, dump_pickle=True, fit_lm=True, model=None,
        show_lm_plot=1, fig_size=(5, 4.5)):
    """looped_experiment.

    This function reads a json file with parameters. That file describes how
    to run the looped experiment. Looped experiments have a for loop that loops
    over a parameter. Each of the GPIO pins can be assigned to measure any
    number of code sections. For each pin the code sections have to alternate
    in the same pattern, but different pins can measure loops with different
    lengths.

    Keyword Arguments:
        config_file {path} -- Path (or list of folders) to the json file with
            parameters (default: {path.abspath(path.join(path.curdir, "looped_experiment.json"))})
        verbose {int} -- How much info to print during the run (default: {1})
        config_dict {dict} -- Parameters for DGILibExtra, in adition to the
            ones in the json file (default: {{}})
        live_plot {bool} -- Whether to show a live plot of the measurement
            (default: {False})
        max_log_time {int} -- Time after which the measurement is broken off
            in case the log_stop_function never returns True (default: {1000})
        log_stop_function {function} -- Function that will be evaluated on the
            collected data. If it returns True the logging will be stopped
            even if the duration has not been reached (default: Stops when all
            GPIO pins are high)
        analysis_stop_function {function} -- Function that will be evaluated on
            the collected data. If it returns True the logging will be stopped
            even if the duration has not been reached (default: Stops when all
            GPIO pins are high)
        drop_last_samples {bool} -- Number of samples to drop from the end of
            the parsed data (default: {0})
        dump_pickle {int} -- Whether to store the results in pickle files (
            default: {True})
        fit_lm {bool} -- Whether to fit a model to the data (default: {True})
        model {Model} -- Model to fit to the data (default: Simple line model
            with slope and intercept)
        show_lm_plot {int} -- Whether to plot the results of the model
            1: Plot single view (default: {1})
            2: Plot each parameter individually and show the residuals
        fig_size {tuple} -- Size of the residuals plot
    """
    # The config_file should be a json file with parameters
    with open(path.join(config_file)) as json_file:
        config = json.load(json_file)

    # Parse project paths. If they start with a dot, assume they are relative
    # to the json file
    project_paths = config.get("project_paths")
    for i, project_path in enumerate(project_paths):
        if project_path[0] == '.':
            project_paths[i] = path.abspath(
                path.join(path.dirname(config_file), *project_path))
        else:
            project_paths[i] = path.abspath(path.join(*project_path))

    # Compile project and program it on the board
    atprogram(project_paths[0],device_name = device_name, clean=False, build=False,
              erase=True, program=False, verify=False, verbose=verbose)
    for project_path in project_paths:
        atprogram(project_path,device_name = device_name, clean=False, build=True, erase=False,
                  program=True, verify=False, verbose=verbose)

    # Load config_dict from config_file
    _config_dict = config.get("config_dict")
    if live_plot:
        _config_dict.update(config.get("config_dict_plot"))
    # Update config_dict from function argument
    _config_dict.update(config_dict)
    config_dict = _config_dict

    # Set the log folder for the csvs
    log_folder = config_dict.get(
        "log_folder", path.join(path.dirname(config_file)))
    config_dict["log_folder"] = log_folder

    # Get stop function for logger
    if log_stop_function is None:
        def log_stop_function(logger_data):
            return len(logger_data.gpio) and all(logger_data.gpio.values[-1])

    if verbose:
        print(f"Starting DGILibExtra with config: \n{config_dict}\n")

    logger_data = LoggerData()
    with DGILibExtra(**config_dict) as dgilib:
        dgilib.device_reset()
        dgilib.logger.log(max_log_time, log_stop_function)

        # Get data from object
        if LOGGER_OBJECT in dgilib.logger.loggers:
            logger_data = dgilib.data
        # Get data from csv files
        elif LOGGER_CSV in dgilib.logger.loggers:
            for interface_id, interface in dgilib.interfaces.items():
                logger_data[interface_id] += interface.csv_read_file(
                    path.join(dgilib.logger.log_folder,
                              (interface.file_name_base + '_' +
                               interface.name + ".csv")))

    if verbose:
        print(f"DGILibExtra logger_data: {logger_data}")

    # Get stop function for analysis
    if analysis_stop_function is None:
        def analysis_stop_function(pin_values):
            return all(pin_values)

    # Parse data
    analysis_config = config.get("analysis")
    result_types = analysis_config.get("result_types")
    x_step = analysis_config.get("x_step")
    parsed_data = {}
    for pin, parameter_names in analysis_config.get("pins").items():
        data = power_and_time_per_pulse(
            logger_data, int(pin), stop_function=analysis_stop_function)
        num_names = len(parameter_names)
        for i, parameter_name in enumerate(parameter_names):
            end_index = -drop_last_samples * num_names or None
            parsed_data[parameter_name] = {
                result_types[0]: data[0][i:end_index:num_names],
                result_types[1]: data[1][i:end_index:num_names],
                "x_step": x_step}
    if dump_pickle:
        pickle.dump(parsed_data, open(
            path.join(path.dirname(config_file),
                      f"{config_dict.get('file_name_base')}_looped.p"), "wb"))

    # Fit lm
    if fit_lm:
        if model is None:
            def line(x, intercept, slope):
                """a line"""
                return [intercept + slope*i for i in x]

            model = Model(line)
            params = model.make_params(intercept=0, slope=1)
        else:
            params = model.params

        model_results = {}
        labels = analysis_config.get("labels")
        for parameter_name in parsed_data.keys():
            length = len(parsed_data[parameter_name][result_types[0]])
            x_step = parsed_data[parameter_name]["x_step"]
            num_bytes = range(x_step, (length+1)*x_step, x_step)
            if verbose:
                print(
                    f"Fitting model to {parameter_name} with {length} " +
                    f"samples, from {min(num_bytes)} to {max(num_bytes)} "
                    f"bytes in steps of {x_step}.")
            model_result = {}
            for result_type in result_types:
                model_result[result_type] = model.fit(
                    parsed_data[parameter_name][result_type], params,
                    x=num_bytes)
                if verbose >= 2:
                    print(model_result[result_type].fit_report())
                # Plot multiple view
                if show_lm_plot >= 2:
                    fig, grid = model_result[result_type].plot(
                        xlabel=f"{labels[result_type]['x']} " +
                               f"[{labels[result_type]['x_unit']}]",
                        ylabel=f"{labels[result_type]['y']} " +
                               f"[{labels[result_type]['y_unit']}]")
                    fig.canvas.set_window_title(
                        f"Residuals of {parameter_name}")
                    fig.tight_layout()
                    fig.show()
                model_results[parameter_name] = model_result

        # Plot single view
        if show_lm_plot:
            import matplotlib.pyplot as plt
            fig = plt.figure(figsize=(9, 6))
            fig.canvas.set_window_title(f"Analysis {config.get('name')}")
            colors = dict(zip(result_types, ['r', 'b']))
            line_styles = (
                line_style for line_style in ('-', '--', '-.', ':') * 2)
            # fig.suptitle(f"Energy analysis of {config.get('name')}")
            ax = {}
            ax[result_types[0]] = fig.add_subplot(1, 1, 1)
            ax[result_types[1]] = ax[result_types[0]].twinx()
            ax[result_types[0]].set_xlabel(
                f"{labels[result_types[0]]['x']} " +
                f"[{labels[result_types[0]]['x_unit']}]")
            for result_type in result_types:
                ax[result_type].set_ylabel(
                    f"{labels[result_type]['y']} " +
                    f"[{labels[result_type]['y_unit']}]",
                    color=colors[result_type])
                ax[result_type].tick_params('y', colors=colors[result_type])

            lines = []
            title_str = ""
            for parameter_name in parsed_data.keys():
                length = len(parsed_data[parameter_name][result_types[0]])
                x_step = parsed_data[parameter_name]["x_step"]
                num_bytes = range(x_step, (length+1)*x_step, x_step)
                model_result = {}
                line_style = next(line_styles)
                for result_type in result_types:
                    label = f"{parameter_name} {labels[result_type]['y']}"
                    lines += ax[result_type].plot(
                        num_bytes, parsed_data[parameter_name][result_type],
                        colors[result_type] + line_style, label=label)
                    title_str += f"{label} "
                    for param in params.keys():
                        title_str += "".join(
                            f"{params[param].name.capitalize()}: ")
                        title_str += "".join(
                            f"{model_results[parameter_name][result_type].params[param].value: .03} ")
                        title_str += "".join(
                            f"{labels[result_type]['y_unit']}, ")
                    title_str = title_str[:-2] + \
                        f" per {labels[result_type]['x_unit']}\n"
            ax[result_types[0]].legend(handles=lines)
            ax[result_types[0]].set_title(title_str[:-1])
            # fig.tight_layout()
            fig.tight_layout(rect=(0.05, 0.05, 1, 1))
            fig.set_size_inches(*fig_size, forward=True)
            #fig.show()

        # Save model results to file
        if dump_pickle:
            model_results_dump = {}
            for parameter_name in model_results.keys():
                model_results_dump[parameter_name] = {}
                for result_type in model_results[parameter_name].keys():
                    model_results_dump[parameter_name][result_type] = \
                        model_results[parameter_name][result_type].values

            pickle.dump(model_results_dump, open(path.join(
                path.dirname(config_file),
                f"{config_dict.get('file_name_base')}_model.p"), "wb"))
