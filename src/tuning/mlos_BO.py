#!/usr/bin/env python
# coding: utf-8

# In[32]:
import plotly.graph_objects as go
from tabulate import tabulate
import mlos_core.optimizers
import ConfigSpace as CS
from ConfigSpace import UniformIntegerHyperparameter
from ConfigSpace import UniformFloatHyperparameter
from ConfigSpace import CategoricalHyperparameter
import pandas as pd
import plotly.express as px
import os
import time
import sys
import logging

# In[33]:

logging.basicConfig(filename='../../build/logs/mlos_logger.txt',
                    filemode='a',
                    format='%(asctime)s,%(msecs)d %(name)s %(levelname)s %(message)s',
                    datefmt='%H:%M:%S',
                    level=logging.INFO)

logging.info("Running mlos")

logger = logging.getLogger('mlosLogger')
pd.set_option('display.max_columns', None)
def cost_model(index):
    performance_data = pd.read_csv('../../build/performance/performance_metrics.csv', header=None, names=["empty_reads", "non_empty_reads", "range_reads", "writes"])
    return performance_data.iloc[index:].mean().sum()


# In[34]:


default_values = {
    "max_background_jobs": 2,
    "max_subcompactions": 1,
    "max_write_buffer_number":2,
    "level0_slowdown_writes_trigger": 20,
    "level0_stop_writes_trigger": 36,
    "target_file_size_base": 64 * 1048576,
    "target_file_size_multiplier": 1,
    "max_bytes_for_level_multiplier": 10,
    "write_buffer_size": 64 << 20,
    "level0_file_num_compaction_trigger": 4,
    "max_bytes_for_level_base": 256 * 1048576,
    "score":0
}


# In[35]:


input_space = CS.ConfigurationSpace(seed=1234)

max_open_files=CS.Integer("max_open_files",(-1,1000),default=-1)
max_total_wal_size=CS.Integer("max_total_wal_size",(0,1000))
delete_obsolete_files_period_micros=CS.Integer("delete_obsolete_files_period_micros",(1,1000 * 60 * 60 * 1000000),default=6 * 60 * 60 * 1000000)

max_background_jobs=CS.Integer("max_background_jobs", (1,20), default=default_values["max_background_jobs"])
max_subcompactions=CS.Integer("max_subcompactions", (1,20), default=default_values["max_subcompactions"])
compaction_readahead_size=CS.Integer("compaction_readahead_size",(0,1000 * 1024 * 1024),default=2 * 1024 * 1024)

writable_file_max_buffer_size=CS.Integer("writable_file_max_buffer_size",(0,1000*1024*1024),default=1024 * 1024)
delayed_write_rate=CS.Integer("delayed_write_rate",(0,1000*1024*1024),default=0)
avoid_flush_during_shutdown=CS.Categorical("avoid_flush_during_shutdown", ["true","false"], default="true") 

max_write_buffer_number=CS.Integer("max_write_buffer_number", (1,20), default=default_values["max_write_buffer_number"])
inplace_update_num_locks=CS.Integer("inplace_update_num_locks",(0,100000),default=10000)
memtable_prefix_bloom_size_ratio=CS.Float("memtable_prefix_bloom_size_ratio",(0.0,1.0),default=0)

memtable_whole_key_filtering=CS.Categorical("memtable_whole_key_filtering",["true","false"], default="false") 
memtable_huge_page_size=CS.Integer("memtable_huge_page_size",(0,1000 * 1024 * 1024),default=0)
arena_block_size=CS.Integer("arena_block_size",(0, 2<<30),default=0)

# compression_per_level=CS.("compression_per_level")
level0_slowdown_writes_trigger=CS.Integer("level0_slowdown_writes_trigger", (1,25), default=default_values["level0_slowdown_writes_trigger"]) 
level0_stop_writes_trigger=CS.Integer("level0_stop_writes_trigger", (1,50), default=default_values["level0_stop_writes_trigger"]) 

target_file_size_base=CS.Integer("target_file_size_base", (1,500 * 1048576), default=default_values["target_file_size_base"])
target_file_size_multiplier=CS.Integer("target_file_size_multiplier", (1,20), default=default_values["target_file_size_multiplier"])
max_bytes_for_level_multiplier=CS.Float("max_bytes_for_level_multiplier", (2,20.0), default=default_values["max_bytes_for_level_multiplier"])

write_buffer_size=CS.Float("write_buffer_size",(20<<20,80<<20),default=default_values["write_buffer_size"])
level0_file_num_compaction_trigger=CS.Integer("level0_file_num_compaction_trigger",(-1,10),default=default_values["level0_file_num_compaction_trigger"])
max_bytes_for_level_base=CS.Integer("max_bytes_for_level_base",(128 * 1048576,256 * 1048576),default=default_values["max_bytes_for_level_base"])

# max_bytes_for_level_multiplier_additional=CS.("max_bytes_for_level_multiplier_additional")
# max_compaction_bytes=CS.Integer("max_compaction_bytes")
# soft_pending_compaction_bytes_limit=CS.Integer("soft_pending_compaction_bytes_limit")

# hard_pending_compaction_bytes_limit=CS.Integer("hard_pending_compaction_bytes_limit")
# compaction_options_universal=CS.("compaction_options_universal")
# compaction_options_fifo=CS.("compaction_options_fifo")

max_sequential_skip_in_iterations=CS.Integer("max_sequential_skip_in_iterations",(0,999999),default=8)
max_successive_merges=CS.Integer("max_successive_merges",(0,1000),default=0)
check_flush_compaction_key_order=CS.Categorical("check_flush_compaction_key_order",["true","false"], default="true") 

paranoid_file_checks=CS.Categorical("paranoid_file_checks",["true","false"], default="false") 
report_bg_io_stats=CS.Categorical("report_bg_io_stats",["true","false"], default="false") 
# ttl=CS.Integer("ttl")

# periodic_compaction_seconds=CS.Integer("periodic_compaction_seconds")
# bottommost_temperature=CS.("bottommost_temperature")
enable_blob_files=CS.Categorical("enable_blob_files",["true","false"], default="false") 

min_blob_size=CS.Integer("min_blob_size",(0,1024),default=0)
# blob_file_size=CS.Integer("blob_file_size")
# blob_compression_type=CS.("blob_compression_type")

enable_blob_garbage_collection=CS.Categorical("enable_blob_garbage_collection",["true","false"], default="false") 
blob_garbage_collection_age_cutoff=CS.Float("blob_garbage_collection_age_cutoff",(0.0,1.0),default=0.25)
blob_garbage_collection_force_threshold=CS.Float("blob_garbage_collection_force_threshold",(0.0,1.0),default=1.00)

blob_compaction_readahead_size=CS.Integer("blob_compaction_readahead_size",(0,999999),default=0)
blob_file_starting_level=CS.Integer("blob_file_starting_level",(0,20),default=0)


# In[36]:


input_space.add_hyperparameters([max_background_jobs,
                                  max_subcompactions,
                                 max_write_buffer_number,
                                  level0_slowdown_writes_trigger,
                                level0_stop_writes_trigger,
                                 target_file_size_base,
                                target_file_size_multiplier,
                                max_bytes_for_level_multiplier,
                                 write_buffer_size,
                                 level0_file_num_compaction_trigger,
                                 max_bytes_for_level_base])
                                 
#                                 max_open_files,
#                                  max_total_wal_size,
##                                  delete_obsolete_files_period_micros,
                               
#                                 compaction_readahead_size,
#                                 writable_file_max_buffer_size,
#                                 delayed_write_rate,
#                                 avoid_flush_during_shutdown,
                                
#                                 inplace_update_num_locks,
#                                 memtable_prefix_bloom_size_ratio,
#                                 memtable_whole_key_filtering,
#                                 memtable_huge_page_size,
#                                 arena_block_size,
                               
                                
#                                 max_sequential_skip_in_iterations,
#                                 max_successive_merges,
#                                 check_flush_compaction_key_order,
#                                 paranoid_file_checks,
#                                 report_bg_io_stats,
#                                 enable_blob_files,
#                                 min_blob_size,
#                                 enable_blob_garbage_collection,
#                                 blob_garbage_collection_age_cutoff,
#                                 blob_garbage_collection_force_threshold,
#                                 blob_compaction_readahead_size,
#                                 blob_file_starting_level])


# In[37]:


#optimizer = mlos_core.optimizers.RandomOptimizer(parameter_space=input_space)

#optimizer = mlos_core.optimizers.FlamlOptimizer(parameter_space=input_space)

optimizer = mlos_core.optimizers.SmacOptimizer(parameter_space=input_space) # , seed=42, n_random_init=20)


# In[38]:


logging.info(f"optimizer {optimizer}")


# In[39]:



def pass_values_to_interface(new_val):
    pipe_path = '../../build/passing_params_pipe'
    if not os.path.exists(pipe_path):
        os.mkfifo(pipe_path)
    pipe_fd = os.open(pipe_path, os.O_RDWR | os.O_TRUNC)
    data =''
    try:
        for x in new_val:
            data+='\n' + x + '=' + str(new_val[x].iloc[0])
        data=data.encode('utf-8')
        os.write(pipe_fd, data)
        sys.stdout.flush()
    except KeyboardInterrupt:
        pass
#     finally:
#         os.close(pipe_fd)


# In[40]:

def read_epochs_pipe():
    while True:
        epochs_pipe_path = '../../build/passing_epochs'
        if os.path.exists(epochs_pipe_path):
            epochs_pipe_fd = os.open(epochs_pipe_path, os.O_RDONLY | os.O_TRUNC)
            try:
                buffer_size = 10000
                buffer = os.read(epochs_pipe_fd, buffer_size).decode('utf-8')
                if buffer:
                    reported_epochs = int(buffer)
                    logging.info(f"Reported epochs: {reported_epochs}")
                    return reported_epochs
            except OSError as e:
                if e.errno == os.errno.EAGAIN or e.errno == os.errno.EWOULDBLOCK:
                    pass
                else:
                    raise
            finally:
                os.close(epochs_pipe_fd)
        else:
            pass
    


# In[42]:

restart_indexes=[]
def run_optimization():
    suggested_value = optimizer.suggest()
    pass_values_to_interface(suggested_value)
    suggested_values_df_table = tabulate(suggested_value, headers='keys', tablefmt='pretty')
    logging.info(f"observations {suggested_values_df_table}")
    delta_df = suggested_value.copy()
    for column in delta_df.columns:
        default_value = default_values[column]
        delta_df[column] = delta_df[column] - default_value
    index=read_epochs_pipe()
    time.sleep(5)
    if index<0:
        restart_indexes.append(-index)
        target_value=float('inf')
    else:
        target_value = cost_model(index)
    optimizer.register(suggested_value, pd.Series([target_value]))


n_iterations = 50
for i in range(n_iterations):
    logging.info(f"epoch:{i}")
    run_optimization()

observations_df = optimizer.get_observations()
observations_df_table = tabulate(observations_df, headers='keys', tablefmt='pretty')

logging.info(f"observations {observations_df_table}")
logging.info(f"restart indexes {restart_indexes}")
# In[ ]:


delta_df = observations_df.copy()
for column in delta_df.columns:
    default_value = default_values[column]
    delta_df[column] = delta_df[column] - default_value


# In[ ]:

delta_df_table = tabulate(delta_df, headers='keys', tablefmt='pretty')
logging.info(f"delat df {delta_df_table}")


# In[ ]:


optimizer.get_best_observation()


# In[ ]:


optimum_value=optimizer.suggest()
pass_values_to_interface(optimum_value)


# In[ ]:


logging.info(f"optimum value reported {tabulate(optimum_value, headers='keys', tablefmt='pretty')}")


# In[ ]:

data_url = '../../build/performance/performance_metrics.csv'
df = pd.read_csv(data_url, header=None, names=["Empty Reads", "Non-Empty Reads", "Range Reads", "Writes"])

# Plot Empty Reads\n",
fig_empty_reads = px.line(df, x=df.index, y="Empty Reads", title="Empty Reads")
fig_non_empty_reads = px.line(df, x=df.index, y="Non-Empty Reads", title="Non-Empty Reads")
fig_range_reads = px.line(df, x=df.index, y="Range Reads", title="Range Reads")
fig_writes = px.line(df, x=df.index, y="Writes", title="Writes")
                     
for restart_index in restart_indexes:
    fig_empty_reads.add_trace(go.Scatter(x=[restart_index], y=[df["Empty Reads"].iloc[restart_index]],
                                         mode='markers', marker=dict(color='red',size=10)))
    fig_non_empty_reads.add_trace(go.Scatter(x=[restart_index], y=[df["Non-Empty Reads"].iloc[restart_index]],
                                            mode='markers', marker=dict(color='red',size=10)))
    fig_range_reads.add_trace(go.Scatter(x=[restart_index], y=[df["Range Reads"].iloc[restart_index]],
                                         mode='markers', marker=dict(color='red',size=10)))
    fig_writes.add_trace(go.Scatter(x=[restart_index], y=[df["Writes"].iloc[restart_index]],
                                    mode='markers', marker=dict(color='red',size=10)))


fig_empty_reads.update_xaxes(title_text='Iterations')
fig_empty_reads.update_yaxes(title_text='Time (ms)')
fig_non_empty_reads.update_xaxes(title_text='Iterations')
fig_non_empty_reads.update_yaxes(title_text='Time (ms)')
fig_range_reads.update_xaxes(title_text='Iterations')
fig_range_reads.update_yaxes(title_text='Time (ms)')
fig_writes.update_xaxes(title_text='Iterations')
fig_writes.update_yaxes(title_text='Time (ms)')

                     
fig_empty_reads.show()
fig_non_empty_reads.show()
fig_range_reads.show()
fig_writes.show()

fig_empty_reads.write_image("../../build/performance/empty_reads_graph.png")
fig_non_empty_reads.write_image("../../build/performance/non_empty_reads_graph.png")
fig_range_reads.write_image("../../build/performance/range_reads_graph.png")
fig_writes.write_image("../../build/performance/writes_graph.png")


# In[ ]:


# hyper_parameters_integer = {
#     "max_open_files":[-1,10000, -1],
#     "max_total_wal_size":[0, 100000, 0],
#     "delete_obsolete_files_period_micros":[0, 6 * 60 * 60 * 1000000, 6 * 60 * 60 * 1000000],
#     "max_background_jobs":[2, 10, 2],
#     "max_subcompactions":[1, 100, 1],
#     "compaction_readahead_size":[0, 1000000, 0],
#     "writable_file_max_buffer_size":[0, 1024 * 1024, 1024 * 1024],
#     "delayed_write_rate":[0, 1000000, 0],
#
#     "max_write_buffer_number":[1, 10, 2],
#     "inplace_update_num_locks":[0, 10000, 0],
#     "memtable_huge_page_size":[0, 1000000, 0],
#     "arena_block_size":[0, 1000000, 0],
#     "level0_slowdown_writes_trigger":[0, 100, 20],
#     "level0_stop_writes_trigger":[0, 100, 36],
#     "target_file_size_base":[0, 100000000, 64 * 1048576],
#     "target_file_size_multiplier":[0, 100, 1],
#     "max_compaction_bytes":[0, 1000000000, 64 * 1048576 * 25],
#     "soft_pending_compaction_bytes_limit":[0, 1000000000, 64 * 1073741824],
#     "hard_pending_compaction_bytes_limit":[0, 1000000000, 256 * 1073741824]
# }
#
# hyper_parameters_float ={
#     "memtable_prefix_bloom_size_ratio":[0.0, 1.0, 0.0],
#     "max_bytes_for_level_multiplier":[0.0, 100.0, 10.0]
# }
#
# hyper_parameters_catagorical ={
#     "avoid_flush_during_shutdown":[[True, False],False],
#     "memtable_whole_key_filtering":[[True, False],False],
#    "compression_per_level":[["kNoCompression", "kSnappyCompression", "kZlibCompression", "kBZip2Compression", "kLZ4Compression", "kLZ4HCCompression", "kXpressCompression", "kZSTD"], "kNoCompression"]
#
# }


# In[ ]:




