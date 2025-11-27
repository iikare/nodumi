import pandas as pd
import matplotlib.pyplot as plt

tick_interval = 50 
date_format = '%Y-%m-%d'

df = pd.read_csv('stats.csv')
df['date'] = pd.to_datetime(df['date'], format='%Y-%m-%d %H:%M:%S %z')

plt.figure(figsize=(10, 6))
plt.plot(df.index, df['loc'], marker='o', linestyle='-')

indices = df.index[::tick_interval]
labels = df['date'][::tick_interval].apply(lambda x: x.strftime(date_format))

plt.xticks(indices, labels, rotation=45, ha='right')
plt.title(f'loc over time')
plt.xlabel(f'date')
plt.ylabel('loc')

plt.grid(True, linestyle='--', alpha=0.7)
plt.tight_layout()

plt.show()


