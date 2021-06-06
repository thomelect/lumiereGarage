EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Connector:Conn_WallSocket_Earth J?
U 1 1 5E4BF1BE
P 2600 2900
F 0 "J?" H 2854 2911 50  0000 L CNN
F 1 "Conn_WallSocket_Earth" H 2854 2820 50  0000 L CNN
F 2 "" H 2300 3000 50  0001 C CNN
F 3 "~" H 2300 3000 50  0001 C CNN
	1    2600 2900
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_WallSocket_Earth J?
U 1 1 5E4BF61E
P 6050 2900
F 0 "J?" H 6304 2911 50  0000 L CNN
F 1 "Conn_WallSocket_Earth" H 6304 2820 50  0000 L CNN
F 2 "" H 5750 3000 50  0001 C CNN
F 3 "~" H 5750 3000 50  0001 C CNN
	1    6050 2900
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_WallSocket_Earth J?
U 1 1 5E4C57B7
P 7650 2900
F 0 "J?" H 7904 2911 50  0000 L CNN
F 1 "Conn_WallSocket_Earth" H 7904 2820 50  0000 L CNN
F 2 "" H 7350 3000 50  0001 C CNN
F 3 "~" H 7350 3000 50  0001 C CNN
	1    7650 2900
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_WallSocket_Earth J?
U 1 1 5E4C57BD
P 9300 2900
F 0 "J?" H 9554 2911 50  0000 L CNN
F 1 "Conn_WallSocket_Earth" H 9554 2820 50  0000 L CNN
F 2 "" H 9000 3000 50  0001 C CNN
F 3 "~" H 9000 3000 50  0001 C CNN
	1    9300 2900
	1    0    0    -1  
$EndComp
$Comp
L Relay:RAYEX-L90 K?
U 1 1 5E4C3230
P 4350 2700
F 0 "K?" V 4953 2459 50  0000 L CNN
F 1 "RAYEX-L90" V 4874 2459 50  0000 L CNN
F 2 "Relay_THT:Relay_SPDT_RAYEX-L90" H 4800 2650 50  0001 L CNN
F 3 "https://a3.sofastcdn.com/attachment/7jioKBjnRiiSrjrjknRiwS77gwbf3zmp/L90-SERIES.pdf" H 4700 3700 50  0001 L CNN
	1    4350 2700
	0    1    1    0   
$EndComp
$Comp
L Switch:SW_SPST SW?
U 1 1 5E4C5023
P 3150 2100
F 0 "SW?" H 3150 2335 50  0000 C CNN
F 1 "SW_SPST" H 3150 2244 50  0000 C CNN
F 2 "" H 3150 2100 50  0001 C CNN
F 3 "~" H 3150 2100 50  0001 C CNN
	1    3150 2100
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_WallPlug_Earth P?
U 1 1 5E4CB7A2
P 1350 2800
F 0 "P?" H 1417 3125 50  0000 C CNN
F 1 "Conn_WallPlug_Earth" H 1417 3034 50  0000 C CNN
F 2 "" H 1750 2800 50  0001 C CNN
F 3 "~" H 1750 2800 50  0001 C CNN
	1    1350 2800
	1    0    0    -1  
$EndComp
Wire Wire Line
	1650 2900 1900 2900
Wire Wire Line
	5850 2900 5600 2900
Wire Wire Line
	5600 2900 5600 3500
Wire Wire Line
	7450 2900 7200 2900
Wire Wire Line
	7200 2900 7200 3500
Wire Wire Line
	7200 3500 5600 3500
Connection ~ 5600 3500
Wire Wire Line
	5600 2100 5600 2700
Wire Wire Line
	7450 2100 7450 2700
Wire Wire Line
	7450 2100 9100 2100
Wire Wire Line
	9100 2100 9100 2700
Connection ~ 7450 2100
Wire Wire Line
	9100 2900 8800 2900
Wire Wire Line
	8800 2900 8800 3500
Wire Wire Line
	8800 3500 7200 3500
Connection ~ 7200 3500
Wire Wire Line
	2100 2100 2950 2100
Wire Wire Line
	1650 2700 2100 2700
Wire Wire Line
	1900 3500 5600 3500
Wire Wire Line
	1900 2900 1900 3500
Wire Wire Line
	2400 2700 2100 2700
Connection ~ 2100 2700
Wire Wire Line
	2400 2900 1900 2900
Connection ~ 1900 2900
Wire Wire Line
	5850 2700 5600 2700
Connection ~ 5600 2100
Wire Wire Line
	5600 2100 7450 2100
Wire Wire Line
	2100 2100 2100 2500
Wire Wire Line
	3350 2100 5150 2100
Wire Wire Line
	2100 2700 2100 2500
Wire Wire Line
	2100 2500 3800 2500
Wire Wire Line
	3800 2500 3800 2900
Wire Wire Line
	3800 2900 3950 2900
Connection ~ 2100 2500
Wire Wire Line
	4750 3000 5150 3000
Wire Wire Line
	5150 3000 5150 2100
Connection ~ 5150 2100
Wire Wire Line
	5150 2100 5600 2100
Wire Notes Line
	1850 1600 10600 1600
Wire Notes Line
	10600 1600 10600 3850
Wire Notes Line
	10600 3850 1850 3850
Wire Notes Line
	1850 3850 1850 1600
$Comp
L power:+5V #PWR?
U 1 1 5E4F620B
P 3950 2350
F 0 "#PWR?" H 3950 2200 50  0001 C CNN
F 1 "+5V" H 3965 2523 50  0000 C CNN
F 2 "" H 3950 2350 50  0001 C CNN
F 3 "" H 3950 2350 50  0001 C CNN
	1    3950 2350
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5E4F6DC0
P 4900 2650
F 0 "#PWR?" H 4900 2400 50  0001 C CNN
F 1 "GND" H 4905 2477 50  0000 C CNN
F 2 "" H 4900 2650 50  0001 C CNN
F 3 "" H 4900 2650 50  0001 C CNN
	1    4900 2650
	1    0    0    -1  
$EndComp
Wire Wire Line
	4750 2500 4900 2500
Wire Wire Line
	4900 2500 4900 2650
Wire Wire Line
	3950 2500 3950 2350
$EndSCHEMATC
