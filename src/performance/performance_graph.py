import pandas as pd
import plotly.express as px
import streamlit as st
import time

# Create Streamlit app
st.title("Real-Time Performance Metrics")

# Read data from the CSV file
data_url = '../../build/performance/performance_metrics.csv'

# Create graphs for Empty Reads, Non-Empty Reads, Range Reads, and Writes
fig_empty_reads = px.line(title='Empty Reads')
fig_non_empty_reads = px.line(title='Non-Empty Reads')
fig_range_reads = px.line(title='Range Reads')
fig_writes = px.line(title='Writes')


# Set x and y axis labels
fig_empty_reads.update_xaxes(title_text='Real Time (s)')
fig_empty_reads.update_yaxes(title_text='Time (ms)')
fig_non_empty_reads.update_xaxes(title_text='Real Time (s)')
fig_non_empty_reads.update_yaxes(title_text='Time (ms)')
fig_range_reads.update_xaxes(title_text='Real Time (s)')
fig_range_reads.update_yaxes(title_text='Time (ms)')
fig_writes.update_xaxes(title_text='Real Time (s)')
fig_writes.update_yaxes(title_text='Time (ms)')

# Display the graphs initially
empty_reads_chart = st.plotly_chart(fig_empty_reads)
non_empty_reads_chart = st.plotly_chart(fig_non_empty_reads)
range_reads_chart = st.plotly_chart(fig_range_reads)
writes_chart = st.plotly_chart(fig_writes)

# Real-time updates
while True:
    # Read data from the CSV file
    df = pd.read_csv(data_url, header=None, names=["Empty Reads", "Non-Empty Reads", "Range Reads", "Writes"])

    # Update the graph data
    fig_empty_reads.data[0].x = df.index
    fig_empty_reads.data[0].y = df["Empty Reads"]
    fig_non_empty_reads.data[0].x = df.index
    fig_non_empty_reads.data[0].y = df["Non-Empty Reads"]
    fig_range_reads.data[0].x = df.index
    fig_range_reads.data[0].y = df["Range Reads"]
    fig_writes.data[0].x = df.index
    fig_writes.data[0].y = df["Writes"]

    # Update the displayed graphs
    empty_reads_chart.plotly_chart(fig_empty_reads)
    non_empty_reads_chart.plotly_chart(fig_non_empty_reads)
    range_reads_chart.plotly_chart(fig_range_reads)
    writes_chart.plotly_chart(fig_writes)

    time.sleep(5)  # Update every 5 seconds
