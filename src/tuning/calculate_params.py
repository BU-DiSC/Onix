import os
import time
import sys

pipe_path = '../../build/passing_params_pipe'
if not os.path.exists(pipe_path):
    os.mkfifo(pipe_path)
pipe_fd = os.open(pipe_path, os.O_RDWR)
try:
    while True:
        option_name = "target_file_size_base"

        if option_name == 'exit':
            break
        for i in range(1, 7):
            option_value = str(i)
            print(option_name,option_value)
            data = (option_name + '=' + option_value + '\n').encode('utf-8')
            os.write(pipe_fd, data)
            sys.stdout.flush()
            time.sleep(30)
except KeyboardInterrupt:
    pass
finally:
    os.close(pipe_fd)

