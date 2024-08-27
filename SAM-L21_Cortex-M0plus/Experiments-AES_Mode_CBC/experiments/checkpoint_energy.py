from os import path, scandir
import json
import pickle
from math import ceil
import numpy as np

from experiments.looped_experiment import looped_experiment
from experiments.repeated_experiment import repeated_experiment

from atprogram.atprogram import get_device_info


class CheckpointEnergy(object):
    looped_config_file = "looped_experiment.json"
    repeated_config_file = "repeated_experiment.json"

    def __init__(self, projects_folder=[path.curdir],
                 security_folder=".", security_projects=None,
                 workload_folder="Workloads", workload_projects=None,
                 text_as_size=False):
        self.projects_folder = projects_folder
        self.security_folder = security_folder
        self.workload_folder = workload_folder
        self.security_projects = security_projects
        self.workload_projects = workload_projects
        if self.security_projects is None:
            self.security_projects = [f.name for f in scandir(
                path.join(*self.projects_folder, self.security_folder))
                if f.is_dir()]
        if self.workload_projects is None:
            self.workload_projects = [f.name for f in scandir(
                path.join(*self.projects_folder, self.workload_folder))
                if f.is_dir()]

        self.workloads_info = None
        self.workloads_size = None
        self.workloads_average = None
        self.workloads_std = None

        self.get_security_energy = self.get_security_energy_function()
        self.get_workload_energy = self.get_workload_energy_function()

        self.text_as_size = text_as_size

    def measure_all_security_energy(self, **kwargs):
        for security_project in self.security_projects:
            self.measure_security_energy(security_project, **kwargs)
        self.get_security_energy = self.get_security_energy_function()

    def measure_all_workload_energy(self, **kwargs):
        for workload_project in self.workload_projects:
            self.measure_workload_energy(workload_project, **kwargs)
        self.get_workload_energy = self.get_workload_energy_function()

    def measure_security_energy(self, security_project, config_file=None,
                                **kwargs):
        looped_experiment(path.join(self.get_config_file_path(
            security_project), config_file or self.looped_config_file), **kwargs)

    def measure_workload_energy(self, workload_project, config_file=None,
                                extract_project_size=True,
                                extract_device_info=True, **kwargs):
        repeated_experiment(path.join(self.get_config_file_path(
            workload_project), config_file or self.repeated_config_file),
            extract_project_size=extract_project_size,
            extract_device_info=extract_device_info, **kwargs)

    def get_security_energy_function(self, config_file=None):
        security_charge = {}
        for security_project in self.security_projects:
            config = self.get_config(security_project, config_file)
            pickle_path_base = path.abspath(path.join(
                *self.projects_folder, self.security_folder, security_project,
                config["config_dict"].get("file_name_base", "log")))
            try:
                security_charge[security_project] = {
                    "parsed_data": pickle.load(
                        open(f"{pickle_path_base}_looped.p", "rb")),
                    "model": pickle.load(
                        open(f"{pickle_path_base}_model.p", "rb"))}
            except FileNotFoundError:
                print(f"Cached result for {security_project} not found. " +
                      f"Tried {pickle_path_base}_[looped|model].p")

        def get_security_energy(security_type, number_of_bytes,
                                energy_parameter="Charge", use_model=False):
            if security_type in security_charge.keys():
                power_dict = {}
                for parameter_name in security_charge[security_type][
                        "parsed_data"].keys():
                    index = ceil(
                        number_of_bytes/security_charge[security_type][
                            "parsed_data"][parameter_name].get("x_step", 1))
                    if index < len(security_charge[security_type][
                            "parsed_data"][parameter_name][energy_parameter]) and not use_model:
                        power_dict[parameter_name] = security_charge[
                            security_type]["parsed_data"][parameter_name][
                                energy_parameter][index]
                    else:
                        power_dict[parameter_name] = security_charge[
                            security_type]["model"][parameter_name][
                                energy_parameter]["intercept"] + \
                            number_of_bytes * \
                            security_charge[security_type]["model"][
                                parameter_name][energy_parameter]["slope"]
                return power_dict
            elif security_type is "None":
                return {security_type: 0}  # No security, so 0 C
            else:
                raise ValueError(
                    f"Security type not recognized. Got {security_type}, but" +
                    f" have {list(security_charge.keys())} and 'None'.")
        return get_security_energy

    def get_workload_energy_function(self, config_file=None,
                                     energy_parameter="Charge"):
        workload_charge = {}
        for workload_project in self.workload_projects:
            config = self.get_config(workload_project, config_file)
            pickle_path_base = path.abspath(path.join(
                *self.projects_folder, self.workload_folder, workload_project,
                config["config_dict"].get("file_name_base", "log")))
            try:
                workload_charge[workload_project] = pickle.load(
                    open(f"{pickle_path_base}_repeated.p", "rb"))
            except FileNotFoundError:
                print(f"Cached result for {workload_project} not found. " +
                      f"Tried {pickle_path_base}_repeated.p")

        def get_workload_energy(workload_type, energy_parameter="Charge"):
            if workload_type in workload_charge.keys():
                power_dict = {}
                for parameter_name in workload_charge[workload_type].keys():
                    power_dict[parameter_name] = workload_charge[
                        workload_type][parameter_name][energy_parameter]
                return power_dict
            else:
                raise ValueError(
                    f"workload type not recognized. Got {workload_type}, but" +
                    f" have {list(workload_charge.keys())} and 'None'.")
        self.get_workload_energy = get_workload_energy

        self.workloads_average = {}
        self.workloads_std = {}
        for workload_project in self.workload_projects:
            self.workloads_average[workload_project] = {}
            self.workloads_std[workload_project] = {}
            workload_energy = self.get_workload_energy(workload_project)
            for section in workload_energy.keys():
                self.workloads_average[workload_project][section] = np.average(
                    workload_energy[section])
                self.workloads_std[workload_project][section] = np.std(
                    workload_energy[section])
        return get_workload_energy

    def get_device_info(self, *args, **kwargs):
        return get_device_info(*args, **kwargs)

    def get_workload_info(self, workload_project, config_file=None, *args,
                          **kwargs):
        config = self.get_config(workload_project, config_file)
        pickle_path_base = path.abspath(path.join(
            *self.projects_folder, self.workload_folder,
            workload_project,
            config["config_dict"].get("file_name_base", "log")))
        return pickle.load(open(f"{pickle_path_base}_info.p", "rb"))

    def get_all_workload_info(self, config_file=None, *args, **kwargs):
        self.workloads_info = {}
        self.workloads_size = {}
        for workload_project in self.workload_projects:
            self.workloads_info[workload_project] = self.get_workload_info(
                workload_project, config_file=config_file, *args, **kwargs)
            self.workloads_size[workload_project] = self.workloads_info[
                workload_project]["size"]
        return self.workloads_info

    def get_workload_size(self, workload_project):
        if self.workloads_size is None:
            self.get_all_workload_info()
        return self.workloads_size[workload_project]

    def get_workload_average(self, workload_project):
        return self.workloads_average[workload_project]

    def get_workload_std(self, workload_project):
        return self.workloads_std[workload_project]

    def get_checkpoint_size(self, workload_project):
        result = 3 + 12 + 1 + 1  # 12 GPR + LR + PC

        checkpoint_regions = (
            "data", "bss") if not self.text_as_size else ("text")

        for project in self.get_workload_size(workload_project).values():
            for size_type, size in project.items():
                if size_type in checkpoint_regions:
                    result += size
        return result

    def get_checkpoint_energy(self, security_type, workload_project,
                              energy_parameter="Charge", use_model=False):
        if security_type == "None":
            return 0
        result = 0
        checkpoint_sections = []
        for section_type, section in \
                self.get_config(security_type)["analysis"]["section_types"].items():
            if section_type in ("load", "store"):
                checkpoint_sections += section

        for security_section_type, security_section in \
                self.get_security_energy(
                    security_type, self.get_checkpoint_size(workload_project),
                    energy_parameter, use_model=use_model).items():
            if security_section_type in checkpoint_sections:
                result += security_section
        return result

    def get_config_file_path(self, project):
        # Get the full path to the config file
        if project in self.security_projects:
            project_folder = self.security_folder
        elif project in self.workload_projects:
            project_folder = self.workload_folder
        else:
            raise ValueError(f"Project not found: {project}")
        return path.abspath(path.join(*self.projects_folder, project_folder,
                                      project))

    def get_config(self, project, config_file=None):
        if project in self.security_projects:
            project_folder = self.security_folder
            config_file = config_file or self.looped_config_file
        elif project in self.workload_projects:
            project_folder = self.workload_folder
            config_file = config_file or self.repeated_config_file
        else:
            raise ValueError(f"Project not found: {project}")
        config_file_path = path.abspath(path.join(
            *self.projects_folder, project_folder, project, config_file))
        if not path.isfile(config_file_path):
            raise IOError(f"Config file not found at {config_file_path}")
        with open(config_file_path) as json_file:
            return json.load(json_file)
