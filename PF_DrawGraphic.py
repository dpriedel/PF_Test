#!/usr/bin/python

"""
    This is a driver module to read and extract the data from a
    PF_Chart data file and then plot it as an svg.
"""

import argparse
import datetime
import itertools
import json
import logging
import numpy as np
import os
import pandas as pd
import random
import sys
import traceback

THE_LOGGER = logging.getLogger()        # use the default 'root' name
THE_LOGGER.setLevel(logging.INFO)
handler = logging.StreamHandler(sys.stderr)
formatter = logging.Formatter('%(asctime)s - %(name)s - %(funcName)s - %(levelname)s - %(message)s')
handler.setFormatter(formatter)
THE_LOGGER.addHandler(handler)

import PF_DrawChart_prices


def Main():

    result = 0

    try:

        args = GetArgs()

        if makes_sense_to_run(args):
            ProcessChartFile(args)
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


def GetArgs():

    parser = argparse.ArgumentParser(description='Draw the graphic for the specified PF_Chart file.')
    parser.add_argument("-f", "--file", action="store", dest="input_file_name_", required=True,
                        help="Path name of file to process.")
    parser.add_argument("-o", "--output-dir", action="store", dest="output_directory_name_", default="/tmp",
                        help="Path name of directory to write output file to. Default is '/tmp'.")
    parser.add_argument("--format", action="store", dest="y_axis_format_", required=True,
                        help="Use 'time' or 'date' for y-axis labels.")
    parser.add_argument("-t", "--trend-lines", action="store", dest="trend_lines_", default="no",
                        help="Draw trend lines on graphic. Default is 'no'. Can be 'data' or 'angle'.")
    parser.add_argument("-n", "--number-cols", type=int, action="store", dest="number_columns_", default=0,
                        help="Maximun number of columns to show in graph. Default is '0' which means 'all'.")
    parser.add_argument("--prices", action="store", dest="prices_file_",
                        help="Path name of file containing 'streamed' price data.")
    parser.add_argument("-l", "--logging", action="store", dest="log_level_", default="warning",
                        help="logging level: info, debug, warning, error, critical. Default is 'warning'.")
    parser.add_argument("-u", "--user", action="store", dest="user_name_", default="data_updater_pg", required=False,
                        help="which DB user to run as. Default is 'data_updater_pg'.")
    parser.add_argument("-d", "--database", action="store", dest="database_name_", default="finance", required=False,
                        help="which DB to connect to. Default is 'finance'.")
    parser.add_argument("-m", "--machine_", action="store", dest="machine_name_", default="localhost", required=False,
                        help="machine name or IP of computer whose database we are using. Default is 'localhost'.")
    parser.add_argument("--DB_port", action="store", dest="DB_port_", default="5432", required=False,
                        help="Postgres port number for source DB. Default: 5432")

    # args = parser.parse_args(["-b2017-04-13", "-e2017-04-13", "-k"])
    # args = parser.parse_args(["-b2017-12-25", "-e2017-12-25"])
    args = parser.parse_args()

    LEVELS = {"debug": logging.DEBUG,
              "info": logging.INFO,
              "warning": logging.WARNING,
              "error": logging.ERROR,
              "critical": logging.CRITICAL,
              "none": 9999}
    log_level = LEVELS.get(args.log_level_, logging.WARNING)
    THE_LOGGER.setLevel(level=log_level)
    if log_level == 9999:
        logging.getLogger().addHandler(logging.NullHandler())

    return args


def makes_sense_to_run(args):

    if not os.path.exists(args.input_file_name_):
        print("Unable to find specified file: %s" % args.input_file_name_)
        return False

    if (args.y_axis_format_ != "date" and args.y_axis_format_ != "time"):
        print("Format: %s must be either: 'date' or 'time'.")
        return False

    if (args.trend_lines_ != "no" and args.trend_lines_ != "data" and args.trend_lines_ != "angle"):
        print("Trend lines: %s must be either: 'no' or 'data' or 'angle'.")
        return False

    return True


def ProcessChartFile(args):
    with open(args.input_file_name_) as json_file:
        chart_data = json.load(json_file)

    openData = []
    closeData = []
    highData = []
    lowData = []
    had_step_back = []
    direction_is_up = []
    x_axis_labels = []

    first_col = None
    if len(chart_data["columns"]) > 0:
        first_col = chart_data["columns"][0]
    else:
        first_col = chart_data["current_column"]

    openning_price = first_col["bottom"] if first_col["direction"] == "up" else first_col["top"]

    for col in chart_data["columns"]:
        lowData.append(float(col["bottom"]))
        highData.append(float(col["top"]))

        if col["direction"] == "up":
            openData.append(float(col["bottom"]))
            closeData.append(float(col["top"]))
            direction_is_up.append(True)
        else:
            openData.append(float(col["top"]))
            closeData.append(float(col["bottom"]))
            direction_is_up.append(False)

        # need to do proper time selection here
        if (args.y_axis_format_ == "date"):
            x_axis_labels.append(datetime.datetime.fromtimestamp(int(col["first_entry"]) / 1e9).date())
        else:
            x_axis_labels.append(datetime.datetime.fromtimestamp(int(col["first_entry"]) / 1e9))
        had_step_back.append(col["had_reversal"])

    lowData.append(float(chart_data["current_column"]["bottom"]))
    highData.append(float(chart_data["current_column"]["top"]))

    if chart_data["current_column"]["direction"] == "up":
        openData.append(float(chart_data["current_column"]["bottom"]))
        closeData.append(float(chart_data["current_column"]["top"]))
        direction_is_up.append(True)
    else:
        openData.append(float(chart_data["current_column"]["top"]))
        closeData.append(float(chart_data["current_column"]["bottom"]))
        direction_is_up.append(False)

    # need to do proper time selection here
    if (args.y_axis_format_ == "date"):
        x_axis_labels.append(datetime.datetime.fromtimestamp(int(chart_data["current_column"]["first_entry"]) / 1e9).date())
    else:
        x_axis_labels.append(datetime.datetime.fromtimestamp(int(chart_data["current_column"]["first_entry"]) / 1e9))

    had_step_back.append(chart_data["current_column"]["had_reversal"])

    # see if we need to grab n most recent columns

    if args.number_columns_ > 0:
        x_axis_labels = x_axis_labels[:-args.number_columns_]
        direction_is_up = direction_is_up[:-args.number_columns_]
        had_step_back = had_step_back[:-args.number_columns_]
        openData = openData[:-args.number_columns_]
        closeData = closeData[:-args.number_columns_]
        highData = highData[:-args.number_columns_]
        lowData = lowData[:-args.number_columns_]

    chart_title = "just testing"

    the_data = {}
    the_data["Date"] = x_axis_labels
    the_data["Open"] = openData
    the_data["High"] = highData
    the_data["Low"] = lowData
    the_data["Close"] = closeData

    the_signals = {}

    # NOTE: this code is 'copied' from my C++ code which ported to Python pretty
    # directly (ignoring syntax)

    signal_data = chart_data["signals"]

    # need to have correct number of values in list
    # must be same as number of rows columns in data
    dt_buys = [np.nan] * len(the_data["Open"])
    tt_buys = [np.nan] * len(the_data["Open"])
    db_sells = [np.nan] * len(the_data["Open"])
    tb_sells = [np.nan] * len(the_data["Open"])
    bullish_tt_buys = [np.nan] * len(the_data["Open"])
    bearish_tb_sells = [np.nan] * len(the_data["Open"])
    cat_buys = [np.nan] * len(the_data["Open"])
    cat_sells = [np.nan] * len(the_data["Open"])
    tt_cat_buys = [np.nan] * len(the_data["Open"])
    tb_cat_sells = [np.nan] * len(the_data["Open"])

    had_dt_buy = 0
    had_tt_buy = 0
    had_db_sell = 0
    had_tb_sell = 0
    had_bullish_tt_buy = 0
    had_bearish_tb_sell = 0
    had_cat_buy = 0
    had_cat_sell = 0
    had_tt_cat_buy = 0
    had_tb_cat_sell = 0

    for col_num, sigs in itertools.groupby(signal_data, lambda s: s["column"]):
        most_important = max(sigs, key=lambda s: int(s["priority"]))

        match most_important["type"]:
            case "dt_buy":
                dt_buys[int(most_important["column"])] = float(most_important["box"])
                had_dt_buy += 1
            case "db_sell":
                db_sells[int(most_important["column"])] = float(most_important["box"])
                had_db_sell += 1
            case "tt_buy":
                tt_buys[int(most_important["column"])] = float(most_important["box"])
                had_tt_buy += 1
            case "tb_sell":
                tb_sells[int(most_important["column"])] = float(most_important["box"])
                had_tb_sell += 1
            case "bullish_tt_buy":
                bullish_tt_buys[int(most_important["column"])] = float(most_important["box"])
                had_bullish_tt_buy += 1
            case "bearish_tb_sell":
                bearish_tb_sells[int(most_important["column"])] = float(most_important["box"])
                had_bearish_tb_sell += 1
            case "catapult_buy":
                cat_buys[int(most_important["column"])] = float(most_important["box"])
                had_cat_buy += 1
            case "catapult_sell":
                cat_sells[int(most_important["column"])] = float(most_important["box"])
                had_cat_sell += 1
            case "ttop_catapult_buy":
                tt_cat_buys[int(most_important["column"])] = float(most_important["box"])
                had_tt_cat_buy += 1
            case "tbot_catapult_sell":
                tb_cat_sells[int(most_important["column"])] = float(most_important["box"])
                had_tb_cat_sell += 1

    the_signals["dt_buys"] = dt_buys if had_dt_buy else []
    the_signals["db_sells"] = db_sells if had_db_sell else []
    the_signals["tt_buys"] = tt_buys if had_tt_buy else []
    the_signals["tb_sells"] = tb_sells if had_tb_sell else []
    the_signals["bullish_tt_buys"] = bullish_tt_buys if had_bullish_tt_buy else []
    the_signals["bearish_tb_sells"] = bearish_tb_sells if had_bearish_tb_sell else []
    the_signals["catapult_buys"] = cat_buys if had_cat_buy else []
    the_signals["catapult_sells"] = cat_sells if had_cat_sell else []
    the_signals["tt_catapult_buys"] = cat_buys if had_cat_buy else []
    the_signals["tb_catapult_sells"] = cat_sells if had_cat_sell else []

    streamed_prices = {}
    streamed_prices["the_time"] = []
    streamed_prices["price"] = []
    streamed_prices["signal_type"] = []

    if args.prices_file_:
        prices = pd.read_csv(args.prices_file_, usecols=[0, 1], parse_dates=[0])

        streamed_prices["the_time"] = prices["date"].astype(np.int64)
        streamed_prices["price"] = prices["close"]
        streamed_prices["signal_type"] = [0] * len(streamed_prices["price"])

        # generate some random 'signals' just for testing purposes.

        for ndx in range(20):
            streamed_prices["signal_type"][random.choice(range(len(streamed_prices["signal_type"])))] = 4
        for ndx in range(20):
            streamed_prices["signal_type"][random.choice(range(len(streamed_prices["signal_type"])))] = 5

    date_time_format = "%Y-%m-%d" if args.y_axis_format_ == "date" else "%H:%M:%S"

    chart_name = MakeChartName(chart_data)
    graphic_file_name = os.path.join(args.output_directory_name_, chart_name)

    PF_DrawChart_prices.DrawChart(the_data, chart_data["current_column"]["reversal_boxes"], direction_is_up,
                                  had_step_back, chart_title, graphic_file_name, date_time_format,
                                  args.trend_lines_, False, float(chart_data["y_min"]),
                                  float(chart_data["y_max"]), float(openning_price), the_signals, streamed_prices)


def MakeChartName(chart_data):
    chart_name = "{}_{}{}X{}_{}.{}".format(chart_data["symbol"], chart_data["fname_box_size"],
                                           ("%" if chart_data["boxes"]["box_scale"] == "percent" else ""),
                                           chart_data["current_column"]["reversal_boxes"],
                                           ("linear" if chart_data["boxes"]["box_scale"] == "linear"
                                            else "percent"), "svg")
    return chart_name


if __name__ == '__main__':
    sys.exit(Main())
