#!/usr/local/python-3.12/bin/python3


"""
    This is a driver module to read and extract the data from a
    PF_Chart data file and then plot it as an svg.
"""

import argparse
import datetime
import functools
import itertools
import json
import logging
import numpy as np
import os
import pandas as pd
import random
import sys
import traceback

from pychartdir import *

# import PF_DrawChart_CD

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
        dest="y_axis_format_",
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
        "-n",
        "--number-cols",
        type=int,
        action="store",
        dest="number_columns_",
        default=0,
        help="Maximun number of columns to show in graph. Default is '0' which means 'all'.",
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


def makes_sense_to_run(args):
    if not os.path.exists(args.input_file_name_):
        print("Unable to find specified file: %s" % args.input_file_name_)
        return False

    if args.y_axis_format_ != "date" and args.y_axis_format_ != "time":
        print("Format: %s must be either: 'date' or 'time'.")
        return False

    if (
        args.trend_lines_ != "no"
        and args.trend_lines_ != "data"
        and args.trend_lines_ != "angle"
    ):
        print("Trend lines: %s must be either: 'no' or 'data' or 'angle'.")
        return False

    return True


def ProcessChartFile(args):
    # define some colors

    RED = 0xFF0000  # for down columns
    GREEN = 0x00FF00  # for up columns
    BLUE = 0x0000FF  # for reversed to up columns
    ORANGE = 0xFFA500  # for reversed to down columns

    if args.prices_file_:
        prices = pd.read_csv(
            args.prices_file_, usecols=[0, 1], parse_dates=[0], names=["time", "close"]
        )
        prices.reset_index(inplace=True, names="row_nbr")
    else:
        prices = pd.DataFrame()

    print(prices.head())

    chart_data = PY_PF_Chart.PY_PF_Chart.MakeChartFromJSONFile(args.input_file_name_)
    # print(up_columns)

    # we want to assign different colors to each of the 4 types
    # of column we can have

    upcol_pd = pd.DataFrame()
    downcol_pd = pd.DataFrame()
    rev_to_up_pd = pd.DataFrame()
    rev_to_down_pd = pd.DataFrame()

    up_columns = chart_data.GetBoxesForColumns(PY_PF_Chart.PF_ColumnFilter.e_up_column)
    upcol_pd = pd.DataFrame(up_columns, columns=["col_nbr", "price"])

    down_columns = chart_data.GetBoxesForColumns(
        PY_PF_Chart.PF_ColumnFilter.e_down_column
    )
    downcol_pd = pd.DataFrame(down_columns, columns=["col_nbr", "price"])

    if chart_data.GetReversalBoxes() == 1:
        reversed_to_up_columns = chart_data.GetBoxesForColumns(
            PY_PF_Chart.PF_ColumnFilter.e_reversed_to_up
        )
        rev_to_up_pd = pd.DataFrame(
            reversed_to_up_columns, columns=["col_nbr", "price"]
        )

    if chart_data.GetReversalBoxes() == 1:
        reversed_to_down_columns_columns = chart_data.GetBoxesForColumns(
            PY_PF_Chart.PF_ColumnFilter.e_reversed_to_down
        )
        rev_to_down_pd = pd.DataFrame(
            reversed_to_down_columns_columns, columns=["col_nbr", "price"]
        )

    first_col = chart_data.GetColumn(0)
    last_col = chart_data.GetColumn(chart_data.GetNumberOfColumns() - 1)

    starting_price = (
        first_col.GetColumnBottom()
        if first_col.GetColumnDirection() == PY_PF_Chart.PY_PF_Column.Direction.e_Up
        else first_col.GetColumnTop()
    )
    ending_price = (
        last_col.GetColumnTop()
        if last_col.GetColumnDirection() == PY_PF_Chart.PY_PF_Column.Direction.e_Up
        else last_col.GetColumnBottom()
    )

    overall_pct_chg = (ending_price - starting_price) / starting_price
    explanation_text = ""
    if chart_data.HasReversedColumns():
        explanation_text = (
            "Orange: 1-step Up then reversal Down. Blue: 1-step Down then reversal Up."
        )

    skipped_columns = max(0, chart_data.GetNumberOfColumns() - args.number_columns_)

    skipped_columns_text = ""
    if skipped_columns > 0:
        skipped_columns_text = " (last {} cols)".format(args.number_columns_)

    chart_title = "\n{}{} X {} for {} {}. Overall % change: {:.00%}{}\nLast change: {:%Y-%m-%d %H:%M:%S}\n{}".format(
        chart_data.GetBoxSize(),
        ("%" if chart_data.IsPercent() else ""),
        chart_data.GetReversalBoxes(),
        chart_data.GetSymbol(),
        ("percent" if chart_data.IsPercent() else ""),
        overall_pct_chg,
        skipped_columns_text,
        datetime.datetime.fromtimestamp(
            round(chart_data.GetLastChangeTimeSeconds().total_seconds())
        ),
        explanation_text,
    )

    x_axis_labels = []

    if args.y_axis_format_ == "date":
        lbl_fmt = "{:%Y-%m-%d}"
    else:
        lbl_fmt = "{:%H:%M:%S}"

    for col in chart_data:
        col_label = lbl_fmt.format(
            datetime.datetime.fromtimestamp(
                round(col.GetColumnBeginTime().total_seconds())
            )
        )
        x_axis_labels.append(col_label)

    if prices.empty:
        c = XYChart((14 * 72), (14 * 72))

        # Set the plotarea at (55, 65) and of size 350 x 300 pixels, with a light grey border (0xc0c0c0).
        # Turn on both horizontal and vertical grid lines with light grey color (0xc0c0c0)
        c.setPlotArea(
            50, 100, (14 * 72 - 50), (14 * 72 - 200), -1, -1, 0xC0C0C0, 0xC0C0C0, -1
        )
    else:
        c = XYChart((14 * 72), (11 * 72))
        c.setPlotArea(
            50, 100, (14 * 72 - 50), (11 * 72 - 200), -1, -1, 0xC0C0C0, 0xC0C0C0, -1
        )

    # Add a legend box at (50, 30) (top of the chart) with horizontal layout. Use 12pt Times Bold Italic
    # font. Set the background andu border color to Transparent.
    # c.addLegend(50, 30, 0, "Times New Roman Bold Italic", 12).setBackground(Transparent)

    # Add a title to the chart using 18pt Times Bold Itatic font.
    c.addTitle(chart_title, "Times New Roman Bold Italic", 18)

    # Add a title to the y axis using 12pt Arial Bold Italic font
    # c.yAxis().setTitle("Length (cm)", "Arial Bold Italic", 12)

    # Add a title to the x axis using 12pt Arial Bold Italic font
    # c.xAxis().setTitle("Weight (kg)", "Arial Bold Italic", 12)

    # Set the axes line width to 3 pixels
    c.xAxis().setWidth(3)
    c.yAxis().setWidth(3)

    # Add an orange (0xff9933) scatter chart layer, using 13 pixel diamonds as symbols
    s_layer1 = c.addScatterLayer(
        upcol_pd.col_nbr, upcol_pd.price, "first chart", Cross2Shape(0.5), 13, GREEN
    )
    if chart_data.GetNumberOfColumns() < 40:
        s_layer1.setSymbolScale([0.15] * len(up_columns), XAxisScale)
    s_layer2 = c.addScatterLayer(
        downcol_pd.col_nbr, downcol_pd.price, "first chart", CircleShape, 13, RED
    )
    if chart_data.GetNumberOfColumns() < 40:
        s_layer2.setSymbolScale([0.15] * len(down_columns), XAxisScale)
    if not rev_to_up_pd.empty:
        s_layer3 = c.addScatterLayer(
            rev_to_up_pd.col_nbr,
            rev_to_up_pd.price,
            "first chart",
            Cross2Shape(0.5),
            13,
            BLUE,
        )
        if chart_data.GetNumberOfColumns() < 40:
            s_layer3.setSymbolScale([0.15] * len(reversed_to_up_columns), XAxisScale)
    if not rev_to_down_pd.empty:
        s_layer4 = c.addScatterLayer(
            rev_to_down_pd.col_nbr,
            rev_to_down_pd.price,
            "first chart",
            CircleShape,
            13,
            ORANGE,
        )
        if chart_data.GetNumberOfColumns() < 40:
            s_layer4.setSymbolScale(
                [0.15] * len(reversed_to_down_columns_columns), XAxisScale
            )

    # Add a green (0x33ff33) scatter chart layer, using 11 pixel triangles as symbols
    # c.addScatterLayer(dataX1, dataY1, "Natural", TriangleSymbol, 11, 0x33ff33)

    c.yAxis().addMark(starting_price, RED)

    yyy = c.xAxis().setLabels(x_axis_labels)
    c.xAxis().setLabelStep(int(chart_data.GetNumberOfColumns() / 40), 0)
    yyy.setFontAngle(45)

    # signal types currently implemented

    dt_buys = []
    tt_buys = []
    db_sells = []
    tb_sells = []
    bullish_tt_buys = []
    bearish_tb_sells = []
    cat_buys = []
    cat_sells = []
    tt_cat_buys = []
    tb_cat_sells = []

    signal_data = chart_data.GetSignals()
    print(len(signal_data), "signals")

    # split out different signal types so each type
    # can be a sepaate chart layer with a distinct symbol
    # (we collect data for both PF_Chart and prices chart)

    for sig in signal_data:
        signal_prices_row_info = prices.time.isin([lbl_fmt.format(datetime.datetime.fromtimestamp( round(sig.GetSignalTimeSecs().total_seconds())))])
        sig_prices_row_row = prices[signal_prices_row_info]

        if not sig_prices_row_row.empty:
            sig_prices_row = sig_prices_row_row.row_nbr.to_list()[0]
        else:
            sig_prices_row = np.nan

        sig_info_for_chart = (
            sig.signal_column_,
            sig_prices_row,
            sig.GetSignalBox(),
            sig.GetSignalPrice(),
        )

        match sig.signal_type_:
            case PY_PF_Chart.PF_SignalType.e_double_top_buy:
                dt_buys.append(sig_info_for_chart)

            case PY_PF_Chart.PF_SignalType.e_triple_top_buy:
                tt_buys.append(sig_info_for_chart)

            case PY_PF_Chart.PF_SignalType.e_double_bottom_sell:
                db_sells.append(sig_info_for_chart)

            case PY_PF_Chart.PF_SignalType.e_triple_bottom_sell:
                tb_sells.append(sig_info_for_chart)

            case PY_PF_Chart.PF_SignalType.e_bullish_tt_buy:
                bullish_tt_buys.append(sig_info_for_chart)

            case PY_PF_Chart.PF_SignalType.e_bearish_tb_sell:
                bearish_tb_sells.append(sig_info_for_chart)

            case PY_PF_Chart.PF_SignalType.e_catapult_buy:
                cat_buys.append(sig_info_for_chart)

            case PY_PF_Chart.PF_SignalType.e_catapult_sell:
                cat_sells.append(sig_info_for_chart)

            case PY_PF_Chart.PF_SignalType.e_ttop_catapult_buy:
                tt_cat_buys.append(sig_info_for_chart)

            case PY_PF_Chart.PF_SignalType.e_tbottom_catapult_sell:
                tb_cat_sells.append(sig_info_for_chart)

    # now load our collected signal data into dataframes for use by
    # the graphics software.

    dt_buys_pd = pd.DataFrame(
        dt_buys, columns=["PF_column", "price_column", "signal_box", "signal_price"]
    )
    tt_buys_pd = pd.DataFrame(
        tt_buys, columns=["PF_column", "price_column", "signal_box", "signal_price"]
    )
    db_sells_pd = pd.DataFrame(
        db_sells, columns=["PF_column", "price_column", "signal_box", "signal_price"]
    )
    tb_sells_pd = pd.DataFrame(
        tb_sells, columns=["PF_column", "price_column", "signal_box", "signal_price"]
    )
    bullish_tt_buys_pd = pd.DataFrame(
        bullish_tt_buys, columns=["PF_column", "price_column", "signal_box", "signal_price"]
    )
    bearish_tb_sells_pd = pd.DataFrame(
        bearish_tb_sells, columns=["PF_column", "price_column", "signal_box", "signal_price"]
    )
    cat_buys_pd = pd.DataFrame(
        cat_buys, columns=["PF_column", "price_column", "signal_box", "signal_price"]
    )
    cat_sells_pd = pd.DataFrame(
        cat_sells, columns=["PF_column", "price_column", "signal_box", "signal_price"]
    )
    tt_cat_buys_pd = pd.DataFrame(
        tt_cat_buys, columns=["PF_column", "price_column", "signal_box", "signal_price"]
    )
    tb_cat_sells_pd = pd.DataFrame(
        tb_cat_sells, columns=["PF_column", "price_column", "signal_box", "signal_price"]
    )

    # define our shapes to be used for each signal type
    # no particular thought given to which is used for which

    dt_buy_sym = SquareShape
    tt_buy_sym = DiamondShape
    db_sell_sym = InvertedTriangleShape
    tb_sell_sym = RightTriangleShape
    bullish_tt_buy_sym = TriangleShape
    bearish_tb_sell_sym = CircleShape
    cat_buy_sym = StarShape(3)
    cat_sell_sym = CrossShape(0.1)
    tt_cat_buy_sym = Cross2Shape(0.1)
    tb_cat_sell_sym = ArrowShape()

    if not prices.empty:
        d = XYChart((14 * 72), (8 * 72))
        d.setPlotArea(
            50, 50, (14 * 72 - 50), (8 * 72 - 200), -1, -1, 0xC0C0C0, 0xC0C0C0, -1
        )
        d.addTitle("Closing prices", "Times New Roman Bold Italic", 18)
        p_layer1 = d.addLineLayer(prices.close.to_list())
        p_x_axis_labels = []

        if not dt_buys_pd.empty:
            x_layer1 = d.addScatterLayer(
                dt_buys_pd.price_column,
                dt_buys_pd.signal_price,
                "first chart",
                dt_buy_sym,
                13,
                GREEN,
            )

        if not tt_buys_pd.empty:
            x_layer1 = d.addScatterLayer(
                tt_buys_pd.price_column,
                tt_buys_pd.signal_price,
                "first chart",
                tt_buy_sym,
                13,
                GREEN,
            )

        if not db_sells_pd.empty:
            x_layer1 = d.addScatterLayer(
                db_sells_pd.price_column,
                db_sells_pd.signal_price,
                "first chart",
                db_sell_sym,
                13,
                RED,
            )

        if not tb_sells_pd.empty:
            x_layer1 = d.addScatterLayer(
                tb_sells_pd.price_column,
                tb_sells_pd.signal_price,
                "first chart",
                tb_sell_sym,
                13,
                RED,
            )

        if not bullish_tt_buys_pd.empty:
            x_layer1 = d.addScatterLayer(
                bullish_tt_buys_pd.price_column,
                bullish_tt_buys_pd.signal_price,
                "first chart",
                bullish_tt_buy_sym,
                13,
                GREEN,
            )

        if not bearish_tb_sells_pd.empty:
            x_layer1 = d.addScatterLayer(
                bearish_tb_sells_pd.price_column,
                bearish_tb_sells_pd.signal_price,
                "first chart",
                bearish_tb_sell_sym,
                13,
                RED,
            )

        if not cat_buys_pd.empty:
            x_layer1 = d.addScatterLayer(
                cat_buys_pd.price_column,
                cat_buys_pd.signal_price,
                "first chart",
                cat_buy_sym,
                13,
                GREEN,
            )

        if not cat_sells_pd.empty:
            x_layer1 = d.addScatterLayer(
                cat_sells_pd.price_column,
                cat_sells_pd.signal_price,
                "first chart",
                cat_sell_sym,
                13,
                RED,
            )

        if not tt_cat_buys_pd.empty:
            x_layer1 = d.addScatterLayer(
                tt_cat_buys_pd.price_column,
                tt_cat_buys_pd.signal_price,
                "first chart",
                tt_cat_buy_sym,
                13,
                GREEN,
            )

        if not tb_cat_sells_pd.empty:
            x_layer1 = d.addScatterLayer(
                tb_cat_sells_pd.price_column,
                tb_cat_sells_pd.signal_price,
                "first chart",
                tb_cat_sell_sym,
                13,
                RED,
            )

        for a in prices.time:
            xxx = lbl_fmt.format(a)
            p_x_axis_labels.append(xxx)
        yyy = d.xAxis().setLabels(p_x_axis_labels)
        d.xAxis().setLabelStep(int(prices.shape[0] / 40), 0)
        yyy.setFontAngle(45)

        combined_chart = MultiChart((14 * 72), (18 * 72))
        combined_chart.addChart(0, 0, c)
        combined_chart.addChart(0, (11 * 72), d)
        combined_chart.makeChart("prices.svg")
        sys.exit()
    # Output the chart
    c.makeChart("scatter.svg")

    sys.exit()

    # the code below is example code for grouping signals by column
    # then finding the highest priority signal in each column and
    # showing only that signal for the column.
    #
    # I'm not doing this filter now...just showing all the signals.
    #
    # for col_num, sigs in itertools.groupby(signal_data, lambda s: s.signal_column_):
    #     most_important = max(
    #         sigs, key=functools.cmp_to_key(PY_PF_Chart.CmpSignalsByPriority)
    #     )
    #
    #     match most_important.signal_type_:
    #         case PY_PF_Chart.PF_SignalType.e_double_top_buy:
    #             dt_buys.append(
    #                 lbl_fmt.format(
    #                     datetime.datetime.fromtimestamp(
    #                         round(most_important.GetSignalTimeSecs().total_seconds())
    #                     )
    #                 )
    #             )
    #
    #         case PY_PF_Chart.PF_SignalType.e_triple_top_buy:
    #             tt_buys.append(
    #                 lbl_fmt.format(
    #                     datetime.datetime.fromtimestamp(
    #                         round(most_important.GetSignalTimeSecs().total_seconds())
    #                     )
    #                 )
    #             )

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

    starting_price = (
        first_col["bottom"] if first_col["direction"] == "up" else first_col["top"]
    )

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
        if args.y_axis_format_ == "date":
            x_axis_labels.append(
                datetime.datetime.fromtimestamp(int(col["first_entry"]) / 1e9).date()
            )
        else:
            x_axis_labels.append(
                datetime.datetime.fromtimestamp(int(col["first_entry"]) / 1e9)
            )
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
    if args.y_axis_format_ == "date":
        x_axis_labels.append(
            datetime.datetime.fromtimestamp(
                int(chart_data["current_column"]["first_entry"]) / 1e9
            ).date()
        )
    else:
        x_axis_labels.append(
            datetime.datetime.fromtimestamp(
                int(chart_data["current_column"]["first_entry"]) / 1e9
            )
        )

    had_step_back.append(chart_data["current_column"]["had_reversal"])

    # see if we need to grab n most recent columns

    if args.number_columns_ > 0:
        x_axis_labels = x_axis_labels[: -args.number_columns_]
        direction_is_up = direction_is_up[: -args.number_columns_]
        had_step_back = had_step_back[: -args.number_columns_]
        openData = openData[: -args.number_columns_]
        closeData = closeData[: -args.number_columns_]
        highData = highData[: -args.number_columns_]
        lowData = lowData[: -args.number_columns_]

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
                bullish_tt_buys[int(most_important["column"])] = float(
                    most_important["box"]
                )
                had_bullish_tt_buy += 1
            case "bearish_tb_sell":
                bearish_tb_sells[int(most_important["column"])] = float(
                    most_important["box"]
                )
                had_bearish_tb_sell += 1
            case "catapult_buy":
                cat_buys[int(most_important["column"])] = float(most_important["box"])
                had_cat_buy += 1
            case "catapult_sell":
                cat_sells[int(most_important["column"])] = float(most_important["box"])
                had_cat_sell += 1
            case "ttop_catapult_buy":
                tt_cat_buys[int(most_important["column"])] = float(
                    most_important["box"]
                )
                had_tt_cat_buy += 1
            case "tbot_catapult_sell":
                tb_cat_sells[int(most_important["column"])] = float(
                    most_important["box"]
                )
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
            streamed_prices["signal_type"][
                random.choice(range(len(streamed_prices["signal_type"])))
            ] = 4
        for ndx in range(20):
            streamed_prices["signal_type"][
                random.choice(range(len(streamed_prices["signal_type"])))
            ] = 5

    date_time_format = "%Y-%m-%d" if args.y_axis_format_ == "date" else "%H:%M:%S"

    chart_name = MakeChartName(chart_data)
    graphic_file_name = os.path.join(args.output_directory_name_, chart_name)

    PF_DrawChart_prices.DrawChart(
        the_data,
        chart_data["current_column"]["reversal_boxes"],
        direction_is_up,
        had_step_back,
        chart_title,
        graphic_file_name,
        date_time_format,
        args.trend_lines_,
        False,
        float(chart_data["y_min"]),
        float(chart_data["y_max"]),
        float(openning_price),
        the_signals,
        streamed_prices,
    )


def MakeChartName(chart_data):
    chart_name = "{}_{}{}X{}_{}.{}".format(
        chart_data["symbol"],
        chart_data["fname_box_size"],
        ("%" if chart_data["boxes"]["box_scale"] == "percent" else ""),
        chart_data["current_column"]["reversal_boxes"],
        ("linear" if chart_data["boxes"]["box_scale"] == "linear" else "percent"),
        "svg",
    )
    return chart_name


if __name__ == "__main__":
    sys.exit(Main())
