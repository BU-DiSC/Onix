import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import collections


def read_metrics(filename):
    metrics_dict = collections.defaultdict(list)
    with open(filename, 'r') as file:
        for line in file:
            parts = line.strip().split(',')
            values = [int(part) for part in parts]
            metrics_dict['Empty Reads Duration'].extend([values[0]])
            metrics_dict['Read Duration'].extend([values[1]])
            metrics_dict['Range Reads Duration'].extend([values[2]])
            metrics_dict['Write Duration'].extend([values[3]])
    return metrics_dict


def update_graph(ax, metric_name, metric_values):
    ax.clear()
    if len(metric_values)>20:
        metric_values=metric_values[-20:]
    ax.plot(metric_values, marker='o')
    ax.set_title(metric_name)
    ax.set_ylabel('Time(ms)')
    ax.grid()
    
def update_graphs(i):
    # Load and update the data from the file
    metrics_dict = read_metrics('../../build/performance/performance_metrics.csv')

    # Update the metric values
    empty_reads_values.extend(metrics_dict['Empty Reads Duration'])
    non_empty_reads_values.extend(metrics_dict['Read Duration'])
    range_queries_values.extend(metrics_dict['Range Reads Duration'])
    write_queries_values.extend(metrics_dict['Write Duration'])

    update_graph(ax_empty_reads, 'Empty Reads', empty_reads_values)
    update_graph(ax_non_empty_reads, 'Non-Empty Reads', non_empty_reads_values)
    update_graph(ax_range_queries, 'Range Queries', range_queries_values)
    update_graph(ax_write_queries, 'Write Queries', write_queries_values)


fig, axs = plt.subplots(2, 2, figsize=(12, 6))
ax_empty_reads, ax_non_empty_reads, ax_range_queries, ax_write_queries = axs.flatten()

# Initialize empty lists to store metric values
metrics_dict = read_metrics('../../build/performance/performance_metrics.csv')
empty_reads_values = metrics_dict['Empty Reads Duration']
non_empty_reads_values = metrics_dict['Read Duration']
range_queries_values = metrics_dict['Range Reads Duration']
write_queries_values = metrics_dict['Write Duration']

try:
    ani = FuncAnimation(fig, update_graphs, interval=5000)
    plt.tight_layout()
    plt.show()
        
        
except KeyboardInterrupt:
     pass
