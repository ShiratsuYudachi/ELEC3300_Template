from math import cos, sin, pi
array = []
def toRadians(degrees):
    return degrees * pi / 180

for i in range(0, 90):
    sinVal = sin(toRadians(i))
    array.append(sinVal)

print(str(array).replace('[', '{').replace(']', '}').replace('},', '},\n'))