import numpy as np
import matplotlib.pyplot as plt

DECIMATION = 10

def qpsk_grid(file_path: str = "./build/rxdata.pcm", output_path: str = "./data.npy"):
    raw = np.fromfile(file_path, dtype=np.int16)
    even_count = raw.size // 2 * 2
    raw = raw[:even_count]
    i = raw[0::2]
    q = raw[1::2]
    z = i + 1j * q

    np.save(output_path, z)

    fig, axes = plt.subplots(2, 5, figsize=(18, 8))
    axes = axes.flatten()

    print(z[:10])

    for offset in range(DECIMATION):
        zk = z[offset::DECIMATION]
        ax = axes[offset]
        
        ax.scatter(zk.real, zk.imag, alpha=0.35, s=1)
        ax.set_title(f'OFFSET = {offset}')
        ax.set_xlabel('I')
        ax.set_ylabel('Q')
        ax.grid(True)
        ax.axis('equal')

    plt.suptitle(f'QPSK созвездия OFFSET (0–{DECIMATION})', fontsize=14)
    plt.tight_layout()
    plt.show()

qpsk_grid()