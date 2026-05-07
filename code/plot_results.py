#!/usr/bin/env python3
import matplotlib.pyplot as plt

# Nom du fichier log généré par le proxy
filename = "sensors.log"

s1_values = []
s2_values = []

# Lecture du fichier
with open(filename, "r") as f:
    for line in f:
        line = line.strip()
        if not line:
            continue
        parts = line.split()
        if len(parts) != 2:
            continue
        s1, s2 = parts
        s1_values.append(float(s1))
        s2_values.append(float(s2))

# Vérification que le fichier contient des données
if not s1_values:
    raise ValueError("Le fichier est vide ou mal formaté")

# Création d'un axe temps artificiel
PERIOD_S = 0.1  # 100 ms
times = [i * PERIOD_S for i in range(len(s1_values))]

# Affichage
plt.figure(figsize=(10, 5))
plt.plot(times, s1_values, label="Capteur 1")
plt.plot(times, s2_values, label="Capteur 2")
plt.xlabel("Temps (s)")
plt.ylabel("Valeur")
plt.title("Simulation des deux capteurs sinusoïdaux")
plt.legend()
plt.grid(True)
plt.show()