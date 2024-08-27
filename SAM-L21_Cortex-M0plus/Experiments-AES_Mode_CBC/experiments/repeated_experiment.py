

from os import path
import pickle
import json

from pydgilib_extra import (
    DGILibExtra, power_and_time_per_pulse, LoggerData, LOGGER_OBJECT,
    LOGGER_CSV)
from atprogram.atprogram import (atprogram, get_device_info, get_project_size)


def repeated_experiment(config_file=path.abspath(
        path.join(path.curdir, "repeated_experiment.json")),
        verbose=1, config_dict={}, live_plot=False, max_log_time=1000,
        log_stop_function=None, analysis_stop_function=None, repetitions=10,
        dump_pickle=True, extract_project_size=True, extract_device_info=True):
    """repeated_experiment.

    This function reads a json file with parameters. That file describes how
    to run the repeated experiment. Repeated experiments will be repeated a
    number of times. Each of the GPIO pins can be assigned to measure any
    number of code sections. For each pin the code sections have to alternate
    in the same pattern, but different pins can measure events with different
    numbers of occurrence.

    Keyword Arguments:
        config_file {path} -- Path (or list of folders) to the json file with
            parameters (default: {path.abspath(path.join(path.curdir, \
                "repeated_experiment.json"))})
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
        repetitions {int} -- Number of repetitions of the measurement (default
            {10})
        dump_pickle {int} -- Whether to store the results in pickle files (
            default: {True})
        extract_project_size {bool} -- Whether extract the project size (
            default: {True})
        extract_device_info {bool} -- Whether extract the device info (default:
            {True})
    """
    # The config_file should be a json file with parameters
    with open(path.join(config_file)) as json_file:
        config = json.load(json_file)

    # Parse project paths. If they start with a dot, assume they are relative
    # to the json file
    project_paths = config.get("project_paths")
    for i, project_path in enumerate(project_paths):
        if project_path[0] == path.curdir:
            project_paths[i] = path.abspath(
                path.join(path.dirname(config_file), *project_path))
        else:
            project_paths[i] = path.abspath(path.join(*project_path))

    if extract_project_size or extract_device_info:
        extract_results = {}
        if extract_project_size:
            extract_results["size"] = {}

    # Compile project and program it on the board
    print(project_paths[0])    
    atprogram(project_paths[0],device_name = "ATSAML21J18B", clean=False, build=False,
              erase=True, program=False, verify=False, verbose=verbose)
    for project_path in project_paths:
        if extract_project_size:
            extract_results["size"][path.basename(path.normpath(
                project_path))] = get_project_size(project_path,device_name = "ATSAML21J18B",
                                                   verbose=verbose)
        #print(project_path)    
        atprogram(project_path, device_name = "ATSAML21J18B", clean=False, build=True, erase=False,
                  program=True, verify=False, verbose=verbose)

    if extract_device_info:
        extract_results["info"] = get_device_info(device_name = "ATSAML21J18B",verbose=verbose)

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

    # Get stop function for analysis
    if analysis_stop_function is None:
        def analysis_stop_function(pin_values):
            return all(pin_values)

    if verbose:
        print(f"Starting DGILibExtra with config: \n{config_dict}\n")

    analysis_config = config.get("analysis")
    result_types = analysis_config.get("result_types")

    # Make dict to store results in
    parsed_data = {}
    for pin, parameter_names in analysis_config.get("pins").items():
        for i, parameter_name in enumerate(parameter_names):
            parsed_data[parameter_name] = {
                result_types[0]: [],
                result_types[1]: []}

    with DGILibExtra(**config_dict) as dgilib:
        for _ in range(repetitions):
            logger_data = LoggerData()
            dgilib.device_reset()
            dgilib.logger.log(max_log_time, log_stop_function)

            # Get data from object
            if LOGGER_OBJECT in dgilib.logger.loggers:
                logger_data = dgilib.data
#                 dgilib.empty_data()
            # Get data from csv files
            elif LOGGER_CSV in dgilib.logger.loggers:
                for interface_id, interface in dgilib.interfaces.items():
                    logger_data[interface_id] += interface.csv_read_file(
                        path.join(dgilib.logger.log_folder,
                                  (interface.file_name_base + '_' +
                                   interface.name + ".csv")))

            for pin, parameter_names in analysis_config.get("pins").items():
                data = power_and_time_per_pulse(
                    logger_data, int(pin),
                    stop_function=analysis_stop_function)
                num_names = len(parameter_names)
                for i, parameter_name in enumerate(parameter_names):
                    if (len(data[0][i::num_names]) > 1 or
                            len(data[1][i::num_names]) > 1):
                        print(
                            "Parsing of measurement data failed, " +
                            f"charge: {data[0][i::num_names]}, " +
                            f"time: {data[1][i::num_names]}")
                    parsed_data[parameter_name][result_types[0]
                                                ] += data[0][i::num_names]
                    parsed_data[parameter_name][result_types[1]
                                                ] += data[1][i::num_names]

            if verbose >= 2:
                print(f"DGILibExtra logger_data: {logger_data}")

    # Parse data
    if dump_pickle:
        pickle.dump(parsed_data, open(
            path.join(path.dirname(config_file),
                      f"{config_dict.get('file_name_base')}_repeated.p"),
            "wb"))
        if extract_project_size or extract_device_info:
            pickle.dump(extract_results, open(
                path.join(path.dirname(config_file),
                          f"{config_dict.get('file_name_base')}_info.p"),
                "wb"))
