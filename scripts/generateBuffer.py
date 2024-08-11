from pyperclip import copy
buffer = []
for i in range(20000):
    buffer.append(36)

cstring = str(buffer).replace('[', '{').replace(']', '}').replace('},', '},\n')
print(cstring)
copy(cstring)