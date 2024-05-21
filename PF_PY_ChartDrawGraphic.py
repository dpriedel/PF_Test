Head: main
Push: origin/main
Help: g?

Untracked (10)
? Eodhd_key.dat
? PF_chart.svg
? boxplot_color.py
? floatingbox.svg
? holidays.cpp
? prices.svg
? python-websocket-server/
? python_charts.py
? scatter.svg
? xxclangd

Unstaged (4)
M EndToEnd_Test.cpp
M PF_DrawGraphic_CD.py
@@ -2,8 +2,8 @@
 
 
 """
-    This is a driver module to read and extract the data from a
-    PF_Chart data file and then plot it as an svg.
+This is a driver module to read and extract the data from a
+PF_Chart data file and then plot it as an svg.
 """
 
 import argparse
@@ -11,6 +11,7 @@ import copy
 import datetime
 import functools
 import itertools
+
 # import json
 import logging
 import numpy as np
@@ -44,7 +45,8 @@ def Main():
         args = GetArgs()
 
         if makes_sense_to_run(args):
-            ProcessChartFile(args)
+            # ProcessChartFile(args)
+            DrawChart(args)
         else:
             print("Unable to create graphic.")
             result = 2
@@ -196,6 +198,16 @@ def makes_sense_to_run(args):
 
     return True
 
+
+def DrawChart(args):
+    my_chart = PY_PF_Chart.PY_PF_Chart()
+    my_chart.LoadChartFromJSONChartFile(args.input_file_name_)
+    my_chart.SavePFChartGraphicToFile(
+        "/tmp/spy.svg", PY_PF_Chart.PY_X_AxisFormat.e_show_date
+    )
+    pass
+
+
 def RemoveTooCloseValues(list, min_distance):
     # input is a list of (column number, y-value) tuples
     final_result = []
@@ -234,10 +246,12 @@ def ProcessChartFile(args):
     else:
         prices = pd.DataFrame()
 
-    print(prices.head())
+    print("prices: ", prices.head())
 
-    chart_data = PY_PF_Chart.PY_PF_Chart.MakeChartFromJSONFile(args.input_file_name_)
-    # print(chart_data)
+    chart_data = PY_PF_Chart.PY_PF_Chart.LoadChartFromJSONChartFile(
+        args.input_file_name_
+    )
+    print("chart from JSON:", chart_data)
 
     # we want to assign different colors to each of the 4 types
     # of column we can have so we will put each into a separate layer
@@ -246,16 +260,29 @@ def ProcessChartFile(args):
     # initialize everything to zeros and then overlay with values
     # for each column type
 
-    upcol_pd = pd.DataFrame(columns=["bottom", "top"], index=range(chart_data.GetNumberOfColumns() - 1))
-    upcol_pd.reset_index(inplace=True, names="col_nbr")
-    downcol_pd = pd.DataFrame(columns=["bottom", "top"], index=range(chart_data.GetNumberOfColumns() - 1))
+    upcol_pd = pd.DataFrame(
+        columns=["data", "col_nbr", "bottom", "top"],
+        index=range(chart_data.GetNumberOfColumns() - 1),
+    )
+    # upcol_pd.reset_index(inplace=True, names="col_nbr")
+    downcol_pd = pd.DataFrame(
+        columns=["bottom", "top"], index=range(chart_data.GetNumberOfColumns() - 1)
+    )
     downcol_pd.reset_index(inplace=True, names="col_nbr")
-    rev_to_up_pd = pd.DataFrame(columns=["bottom", "top"], index=range(chart_data.GetNumberOfColumns() - 1))
+    rev_to_up_pd = pd.DataFrame(
+        columns=["bottom", "top"], index=range(chart_data.GetNumberOfColumns() - 1)
+    )
     rev_to_up_pd.reset_index(inplace=True, names="col_nbr")
-    rev_to_down_pd = pd.DataFrame(columns=["bottom", "top"], index=range(chart_data.GetNumberOfColumns() - 1))
+    rev_to_down_pd = pd.DataFrame(
+        columns=["bottom", "top"], index=range(chart_data.GetNumberOfColumns() - 1)
+    )
     rev_to_down_pd.reset_index(inplace=True, names="col_nbr")
 
-    up_columns = chart_data.GetTopBottomForColumns(PY_PF_Chart.PF_ColumnFilter.e_up_column)
+    up_columns = chart_data.GetTopBottomForColumns(
+        PY_PF_Chart.PF_ColumnFilter.e_up_column
+    )
+    up_columns["col_nbr"] = up_columns[:]["data"].col_nbr
+    print("up columns: ", up_columns[0].col_nbr, up_columns[0].col_top)
     xx = pd.DataFrame(up_columns, columns=["col_nbr", "bottom", "top"])
     xx.set_index("col_nbr", drop=False, inplace=True)
     upcol_pd.loc[upcol_pd.col_nbr.isin(xx.col_nbr)] = xx[["col_nbr", "bottom", "top"]]
@@ -265,7 +292,9 @@ def ProcessChartFile(args):
     )
     xx = pd.DataFrame(down_columns, columns=["col_nbr", "bottom", "top"])
     xx.set_index("col_nbr", drop=False, inplace=True)
-    downcol_pd.loc[downcol_pd.col_nbr.isin(xx.col_nbr)] = xx[["col_nbr", "bottom", "top"]]
+    downcol_pd.loc[downcol_pd.col_nbr.isin(xx.col_nbr)] = xx[
+        ["col_nbr", "bottom", "top"]
+    ]
 
     # reversal columns are only possible if the number of reversal boxes is 1
 
@@ -275,15 +304,21 @@ def ProcessChartFile(args):
         )
         xx = pd.DataFrame(reversed_to_up_columns, columns=["col_nbr", "bottom", "top"])
         xx.set_index("col_nbr", drop=False, inplace=True)
-        rev_to_up_pd.loc[rev_to_up_pd.col_nbr.isin(xx.col_nbr)] = xx[["col_nbr", "bottom", "top"]]
+        rev_to_up_pd.loc[rev_to_up_pd.col_nbr.isin(xx.col_nbr)] = xx[
+            ["col_nbr", "bottom", "top"]
+        ]
 
     if chart_data.GetReversalBoxes() == 1:
         reversed_to_down_columns = chart_data.GetTopBottomForColumns(
             PY_PF_Chart.PF_ColumnFilter.e_reversed_to_down
         )
-        xx = pd.DataFrame(reversed_to_down_columns, columns=["col_nbr", "bottom", "top"])
+        xx = pd.DataFrame(
+            reversed_to_down_columns, columns=["col_nbr", "bottom", "top"]
+        )
         xx.set_index("col_nbr", drop=False, inplace=True)
-        rev_to_down_pd.loc[rev_to_down_pd.col_nbr.isin(xx.col_nbr)] = xx[["col_nbr", "bottom", "top"]]
+        rev_to_down_pd.loc[rev_to_down_pd.col_nbr.isin(xx.col_nbr)] = xx[
+            ["col_nbr", "bottom", "top"]
+        ]
 
     first_col = chart_data.GetColumn(0)
     last_col = chart_data.GetColumn(chart_data.GetNumberOfColumns() - 1)
@@ -373,12 +408,10 @@ def ProcessChartFile(args):
     c.yAxis().setWidth(3)
 
     # Add an orange (0xff9933) scatter chart layer, using 13 pixel diamonds as symbols
-    s_layer1 = c.addBoxLayer(
-        upcol_pd.top, upcol_pd.bottom, GREEN, "Up")
+    s_layer1 = c.addBoxLayer(upcol_pd.top, upcol_pd.bottom, GREEN, "Up")
     # if chart_data.GetNumberOfColumns() < 40:
     #     s_layer1.setSymbolScale([0.15] * len(up_columns), XAxisScale)
-    s_layer2 = c.addBoxLayer(
-        downcol_pd.top, downcol_pd.bottom, RED, "Down")
+    s_layer2 = c.addBoxLayer(downcol_pd.top, downcol_pd.bottom, RED, "Down")
     # if chart_data.GetNumberOfColumns() < 40:
     #     s_layer2.setSymbolScale([0.15] * len(down_columns), XAxisScale)
     if not rev_to_up_pd.empty:
@@ -548,7 +581,9 @@ def ProcessChartFile(args):
         )
         d.addTitle("Closing prices", "Times New Roman Bold Italic", 18)
         p_layer1 = d.addLineLayer(prices.close.to_list())
-        d.addLegend(50, 30, 0, "Times New Roman Bold Italic", 12).setBackground(Transparent)
+        d.addLegend(50, 30, 0, "Times New Roman Bold Italic", 12).setBackground(
+            Transparent
+        )
 
         p_x_axis_labels = []
 
M Unit_Test.cpp
M makefile_e2e

Unpushed to origin/main (1)
dd14a9d chore: gcc-14 link fix, new boost and gcc
