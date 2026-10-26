import sys
import matplotlib.pyplot as plt
import pandas as pd

use_date_spacing = False
if len(sys.argv) > 1:
  arg = sys.argv[1].strip().lower()
  if arg == "y":
    use_date_spacing = True
  elif arg == "n":
    use_date_spacing = False
  else:
    print(f"Unknown argument '{sys.argv[1]}'. Please use 'y' or 'n'.")
    print("Usage: python stats.py [y/n]")
    sys.exit(1)
else:
  print("Usage: python stats.py [y/n]")
  print("Defaulting to 'n' (even index spacing)...")

df = pd.read_csv("stats.csv")
df["date"] = pd.to_datetime(df["date"], utc=True)

plt.figure(figsize=(10, 6))

if use_date_spacing:
  plt.plot(df["date"], df["loc"], marker="o", linestyle="-")
else:
  tick_interval = 50
  date_format = "%Y-%m-%d"

  plt.plot(df.index, df["loc"], marker="o", linestyle="-")

  indices = df.index[::tick_interval]
  labels = df["date"][::tick_interval].dt.strftime(date_format)
  plt.xticks(indices, labels, rotation=45, ha="right")

plt.title("loc over time")
plt.xlabel("date")
plt.ylabel("loc")
plt.grid(True, linestyle="--", alpha=0.7)
plt.tight_layout()

plt.show()
