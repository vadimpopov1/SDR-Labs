import numpy as np
import matplotlib.pyplot as plt

# Открываем файл для чтения
name = "./build/rxdata.pcm"

data = []
imag = []
real = []
counter = 0
count = []
rx_int = []
r = 0
im = 0
with open(name, "rb") as f:
    index = 0
    while (byte := f.read(2)):
        if(index %2 == 0):
            real.append(int.from_bytes(byte, byteorder='little', signed=True))
            r = int.from_bytes(byte, byteorder='little', signed=True)
            counter += 1
            count.append(counter)
        else:
            imag.append(int.from_bytes(byte, byteorder='little', signed=True))
            im = int.from_bytes(byte, byteorder='little', signed=True)
        index += 1
        


iq = np.array(real, dtype=float) + 1j*np.array(imag, dtype=float)

plt.figure(1)
# plt.subplot(2,1,1)
# plt.plot(count,np.abs(imag),color='red')  # Используем scatter для диаграммы созвездия
# plt.plot(count,np.abs(real), color='blue')  # Используем scatter для диаграммы созвездия
plt.plot(count,imag,color='red')  # Используем scatter для диаграммы созвездия
plt.plot(count,real, color='blue')  # Используем scatter для диаграммы созвездия
# plt.subplot(2,1,2)
plt.plot(count, np.abs(iq))
plt.show()