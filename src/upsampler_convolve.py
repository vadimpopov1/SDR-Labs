import numpy as np
import matplotlib.pyplot as plt

N = 10 # Sample Per Symbol
H = np.ones(N, dtype=np.int64) # Последовательность единиц
            
def upsample_zeros(file_path: str = "./data.npy", output_path: str = "./upsampled.npy"):
    data = np.load(file_path)

    out = np.zeros(data.size * N, dtype=data.dtype)
    out[::N] = data

    np.save(output_path, out)


def convolve(file_path: str = "./upsampled.npy", output_path: str = "./convolved.npy"):
    upsampled = np.load(file_path)
    total_samples = upsampled.size

    i = upsampled.real.astype(np.int64)
    q = upsampled.imag.astype(np.int64)

    out_i = np.zeros(total_samples, dtype=np.int64)
    out_q = np.zeros(total_samples, dtype=np.int64)

    for n in range(total_samples):
        acc_i = 0
        acc_q = 0

        for k in range(N):
            if n >= k:
                past_idx = n - k
                acc_i += i[past_idx] * H[k]
                acc_q += q[past_idx] * H[k]

        out_i[n] = acc_i
        out_q[n] = acc_q

    out = out_i + 1j * out_q
    np.save(output_path, out)

upsample_zeros()
convolve()

