import pandas as pd
import plotly.express as px
import streamlit as st
import time

st.title("Real-Time Performance Metrics")

# Read data from the CSV file
data_url = '../../build/performance/performance_metrics.csv'

fig_empty_reads = px.line(title='Empty Reads')
fig_non_empty_reads = px.line(title='Non-Empty Reads')
fig_range_reads = px.line(title='Range Reads')
fig_writes = px.line(title='Writes')

fig_empty_reads.update_xaxes(title_text='Epochs')
fig_empty_reads.update_yaxes(title_text='Time (ms)')
fig_non_empty_reads.update_xaxes(title_text='Epochs')
fig_non_empty_reads.update_yaxes(title_text='Time (ms)')
fig_range_reads.update_xaxes(title_text='Epochs')
fig_range_reads.update_yaxes(title_text='Time (ms)')
fig_writes.update_xaxes(title_text='Epochs')
fig_writes.update_yaxes(title_text='Time (ms)')

# Display the graphs initially
empty_reads_chart = st.plotly_chart(fig_empty_reads)
non_empty_reads_chart = st.plotly_chart(fig_non_empty_reads)
range_reads_chart = st.plotly_chart(fig_range_reads)
writes_chart = st.plotly_chart(fig_writes)

while True:
    df = pd.read_csv(data_url, header=None, names=["Empty Reads", "Non-Empty Reads", "Range Reads", "Writes"])
    epoch_data = df.iloc[-25:]

    # Update the graph data
    fig_empty_reads.data[0].x = epoch_data.index
    fig_empty_reads.data[0].y = epoch_data["Empty Reads"]
    fig_non_empty_reads.data[0].x = epoch_data.index
    fig_non_empty_reads.data[0].y = epoch_data["Non-Empty Reads"]
    fig_range_reads.data[0].x = epoch_data.index
    fig_range_reads.data[0].y = epoch_data["Range Reads"]
    fig_writes.data[0].x = epoch_data.index
    fig_writes.data[0].y = epoch_data["Writes"]

    empty_reads_chart.plotly_chart(fig_empty_reads)
    non_empty_reads_chart.plotly_chart(fig_non_empty_reads)
    range_reads_chart.plotly_chart(fig_range_reads)
    writes_chart.plotly_chart(fig_writes)

    time.sleep(5)
