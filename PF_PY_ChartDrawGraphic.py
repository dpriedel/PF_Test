#!/usr/local/python-3.12/bin/python3


"""
This is a stand-alone program which will construct a PY_PF_Chart
from the supplied JSON Chart file and then generate and store
an SVG image of that chart in the specified output directory.

"""

# %%
import argparse
import logging
import os
import sys
import traceback

THE_LOGGER = logging.getLogger()  # use the default 'root' name
THE_LOGGER.setLevel(logging.INFO)
handler = logging.StreamHandler(sys.stderr)
formatter = logging.Formatter(
    "%(asctime)s - %(name)s - %(funcName)s - %(levelname)s - %(message)s"
)
handler.setFormatter(formatter)
THE_LOGGER.addHandler(handler)

sys.path.insert(0, os.path.expanduser("~/projects/PF_Project/PY_PF_Chart"))
import PY_PF_Chart

# %%


def Main():
    result = 0

    try:
        args = GetArgs()

        if makes_sense_to_run(args):
            DrawChart(args)
        else:
            print("Unable to create graphic.")
            result = 2

    # parse_args will throw a SystemExit after displaying help info.

    except SystemExit:
        result = 7
        pass

    except Exception:
        traceback.print_exc()
        sys.stdout.flush()
        result = 6

    return result


# %%


def GetArgs():
    parser = argparse.ArgumentParser(
        description="Draw the graphic for the specified PF_Chart file."
    )
    parser.add_argument(
        "-f",
        "--file",
        action="store",
        dest="input_file_name_",
        required=True,
        help="Path name of file to process.",
    )
    parser.add_argument(
        "-o",
        "--output-dir",
        action="store",
        dest="output_directory_name_",
        default="/tmp",
        help="Path name of directory to write output file to. Default is '/tmp'.",
    )
    parser.add_argument(
        "--format",
        action="store",
        dest="x_axis_format_",
        required=True,
        help="Use 'time' or 'date' for y-axis labels.",
    )
    parser.add_argument(
        "-t",
        "--trend-lines",
        action="store",
        dest="trend_lines_",
        default="no",
        help="Draw trend lines on graphic. Default is 'no'. Can be 'data' or 'angle'.",
    )
    parser.add_argument(
        "--prices",
        action="store",
        dest="prices_file_",
        help="Path name of file containing 'streamed' price data.",
    )
    parser.add_argument(
        "-l",
        "--logging",
        action="store",
        dest="log_level_",
        default="warning",
        help="logging level: info, debug, warning, error, critical. Default is 'warning'.",
    )
    parser.add_argument(
        "-u",
        "--user",
        action="store",
        dest="user_name_",
        default="data_updater_pg",
        required=False,
        help="which DB user to run as. Default is 'data_updater_pg'.",
    )
    parser.add_argument(
        "-d",
        "--database",
        action="store",
        dest="database_name_",
        default="finance",
        required=False,
        help="which DB to connect to. Default is 'finance'.",
    )
    parser.add_argument(
        "-m",
        "--machine_",
        action="store",
        dest="machine_name_",
        default="localhost",
        required=False,
        help="machine name or IP of computer whose database we are using. Default is 'localhost'.",
    )
    parser.add_argument(
        "--DB_port",
        action="store",
        dest="DB_port_",
        default="5432",
        required=False,
        help="Postgres port number for source DB. Default: 5432",
    )

    # args = parser.parse_args(["-b2017-04-13", "-e2017-04-13", "-k"])
    # args = parser.parse_args(["-b2017-12-25", "-e2017-12-25"])
    args = parser.parse_args()

    LEVELS = {
        "debug": logging.DEBUG,
        "info": logging.INFO,
        "warning": logging.WARNING,
        "error": logging.ERROR,
        "critical": logging.CRITICAL,
        "none": 9999,
    }
    log_level = LEVELS.get(args.log_level_, logging.WARNING)
    THE_LOGGER.setLevel(level=log_level)
    if log_level == 9999:
        logging.getLogger().addHandler(logging.NullHandler())

    return args


# %%


def makes_sense_to_run(args):
    if not os.path.exists(args.input_file_name_):
        print("Unable to find specified PY_Chart JSON file: %s" % args.input_file_name_)
        return False

    if not os.path.exists(args.output_directory_name_):
        print(
            "Unable to find specified output directory: %s"
            % args.output_directory_name_
        )
        return False

    if args.x_axis_format_ != "date" and args.x_axis_format_ != "time":
        print("Format: %s must be either: 'date' or 'time'." % args.x_axis_format_)
        return False

    return True


# %%


def DrawChart(args):
    # construct output file name based on input file name

    graphic_file_name = os.path.basename(args.input_file_name_)
    graphic_file_name = os.path.join(
        args.output_directory_name_, os.path.splitext(graphic_file_name)[0] + ".svg"
    )

    x_axis_scale = (
        PY_PF_Chart.PF_X_AxisFormat.e_show_date
        if args.x_axis_format_ == "date"
        else PY_PF_Chart.PF_X_AxisFormat.e_show_time
    )

    my_chart = PY_PF_Chart.PY_PF_Chart()

    DoDrawGraphic(my_chart, args.input_file_name_, graphic_file_name, x_axis_scale)


# %%


def DoDrawGraphic(pf_chart, json_file_name, graphic_file_name, x_axis_scale):
    PY_PF_Chart.PY_PF_Chart.LoadChartFromJSONChartFile(pf_chart, json_file_name)
    pf_chart.SavePFChartGraphicToFile(graphic_file_name, x_axis_scale)


# %%

if __name__ == "__main__":
    sys.exit(Main())
# %%
