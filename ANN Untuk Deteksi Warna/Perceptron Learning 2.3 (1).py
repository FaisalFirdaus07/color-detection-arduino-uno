import numpy as np

#   Dataset RGB (diperbesar + variasi intensitas)
def gen_color(r, g, b, noise=30):
    """Generate variasi warna dengan noise ±30"""
    data = []
    for _ in range(20):
        rn = np.clip(r + np.random.randint(-noise, noise), 0, 255)
        gn = np.clip(g + np.random.randint(-noise, noise), 0, 255)
        bn = np.clip(b + np.random.randint(-noise, noise), 0, 255)
        data.append([rn, gn, bn])
    return np.array(data)

# Warna dasar
red_base   = gen_color(255, 20, 20)
green_base = gen_color(20, 255, 20)
blue_base  = gen_color(20, 20, 255)

# Gabungkan dataset
X = np.vstack([red_base, green_base, blue_base])

# Label one-hot  (0=R, 1=G, 2=B)
y = np.array(
    [[1, 0, 0]] * len(red_base) +
    [[0, 1, 0]] * len(green_base) +
    [[0, 0, 1]] * len(blue_base)
)

# Normalisasi
X = X / 255.0

#   Parameter model
np.random.seed(42)
w = np.random.rand(3, 3)
b = np.random.rand(3)
lr = 0.05
epochs = 2000

#   Softmax
def softmax(z):
    e = np.exp(z - np.max(z))
    return e / e.sum()

#   Training loop
for epoch in range(epochs):
    for i in range(len(X)):
        z = np.dot(X[i], w) + b
        y_pred = softmax(z)

        error = y[i] - y_pred

        # Update bobot
        w += lr * np.outer(X[i], error)
        b += lr * error

    if epoch % 200 == 0:
        loss = -np.mean(y * np.log(y_pred + 1e-9))
        print("Epoch:", epoch, "Loss:", loss)


#   Simpan hasil bobot
np.savetxt("weights.txt", w)
np.savetxt("bias.txt", b)

print("Training selesai!")
print("Bobot dan bias disimpan ke file.")
