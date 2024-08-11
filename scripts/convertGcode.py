import os
import numpy as np

path = os.popen('pwd').read().strip()+'/'

is2D = True if input('Is this a 2D file? (y/n): ') == 'y' else False
Xscale = 1#10.0
Yscale = 1#2.0
Zscale = 2

F_MAX = 600


generatedArray = []

lastG = -1
lastX = 0
lastY = 0
lastZ = 0
lastF = F_MAX


G = -1
X = 0
Y = 0
Z = 0
F = F_MAX

def matchFirstFloat(line) -> str:
    matched = ''
    for char in line:
        if (char.isdigit() or char == '.' or char == '-'):
            matched += char
        else:
            break
    return matched

def parseLine(line):
    global G
    global X
    global Y
    global Z
    global F
    global F_MAX
    global lastG
    global lastX
    global lastY
    global lastZ
    global lastF

    if ('G' in line):
        lastG = G
        G = float(matchFirstFloat(line.split('G')[-1]))
    if ('X' in line):
        lastX = X
        X = float(matchFirstFloat(line.split('X')[-1])) * Xscale
    if ('Y' in line):
        lastY = Y
        Y = float(matchFirstFloat(line.split('Y')[-1])) * Yscale
    if ('Z' in line):
        lastZ = Z
        Z = float(matchFirstFloat(line.split('Z')[-1])) * Zscale
    if ('F' in line):
        lastF = F
        F = min(float(matchFirstFloat(line.split('F')[-1])), F_MAX)


with open(path+'scripts/test.gcode', 'r') as f:
    lines = f.readlines()
    for line in lines:
        parseLine(line.strip())
        if (G == 0):
            if (is2D):
                generatedArray.append([lastX, lastY, 5, F])
                generatedArray.append([X, Y, 5, F])
                generatedArray.append([X, Y, Z, F])
            else:
                generatedArray.append([X, Y, Z, F])
        if (G == 1):
            generatedArray.append([X, Y, Z, F])

def prettyPrint(array):
    for line in array:
        print(line)

def calculateCenterOfMass(array):
    total_weight = 0
    weighted_sum_x = 0
    weighted_sum_y = 0
    weighted_sum_z = 0

    for i in range(len(array) - 1):
        # 计算两点之间的距离
        dx = array[i+1][0] - array[i][0]
        dy = array[i+1][1] - array[i][1]
        dz = array[i+1][2] - array[i][2]
        distance = (dx**2 + dy**2 + dz**2)**0.5

        # 计算中点坐标
        mid_x = (array[i][0] + array[i+1][0]) / 2
        mid_y = (array[i][1] + array[i+1][1]) / 2
        mid_z = (array[i][2] + array[i+1][2]) / 2

        # 使用距离作为权重计算加权坐标
        weighted_sum_x += mid_x * distance
        weighted_sum_y += mid_y * distance
        weighted_sum_z += mid_z * distance
        total_weight += distance

    # 计算加权平均坐标
    if total_weight == 0:
        return 0, 0, 0  # 避免除以零的错误

    center_x = weighted_sum_x / total_weight
    center_y = weighted_sum_y / total_weight
    center_z = weighted_sum_z / total_weight

    return center_x, center_y, center_z




carr = str(generatedArray).replace('[', '{').replace(']', '}').replace('},', '},\n')

centerOfMassStr = str(calculateCenterOfMass(generatedArray)).replace('(', '{').replace(')', '}')
print(centerOfMassStr)
with open('Core/Inc/gcode.h', 'w') as f:
    f.write(f'''#ifndef __GCODE_H
#define __GCODE_H 	
const float gcodeCenterOfMass[3] = {centerOfMassStr};
const int gcodeLength = {len(generatedArray)};
const float gcode[{len(generatedArray)}][4] = {carr};
#endif''')
    
