import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import numpy as np

df1=pd.read_csv('1.csv')
df1.head()

# max_threshold = (df.loc(1)).quantile(0.95)
# min_threshold = (df.loc(1)).quantile(0.05)
max_threshold = df1['Ref'].quantile(0.95)
min_threshold = df1['Ref'].quantile(0.05)

arr=[]
values=[]
timestamps=[]
arr = df1.to_numpy()

for i in range(0, 1641):
    if(arr[i][1] < max_threshold and arr[i][1] > min_threshold):
        values.append(arr[i][1])
        timestamps.append(arr[i][0])

# for i in range(0, len(values)):
#     print(timestamps[i])
#     print(values[i])
size=[2]
plt.scatter(timestamps, values, s= size)
plt.show()


#plt.plot(df.loc(0), df[(df.loc(1) < max_threshold) & (df.loc(1) > min_threshold)])

# plt.figure(figsize=(12, 4), dpi=200)
# sns.scatterplot(x='Time', y='Ref', data=df[(df['Ref'] < max_threshold) & (df['Ref'] > min_threshold)])


# df_plot = pd.DataFrame(data=df[(df['Ref'] < max_threshold) & (df['Ref'] > min_threshold)])
# df_plot.plot.scatter(x='Time', y='Ref');

# df_plot = pd.DataFrame(data=df)
# df_plot.plot.scatter(x='Time', y='Ref');