import rocksdb
import random
import string

# Define the number of key-value pairs to generate
num_pairs = 10000  # Adjust this number as needed

# Initialize RocksDB
db_path = "rocksdb_data"
db = rocksdb.DB(db_path, rocksdb.Options(create_if_missing=True))

# Function to generate a random key
def generate_random_key():
    key_length = random.randint(5, 15)  # Random key length between 5 and 15 characters
    key = ''.join(random.choices(string.ascii_letters + string.digits, k=key_length))
    return key.encode('utf-8')

# Function to generate a random value
def generate_random_value():
    value_length = random.randint(10, 100)  # Random value length between 10 and 100 characters
    value = ''.join(random.choices(string.ascii_letters + string.digits, k=value_length))
    return value.encode('utf-8')

# Generate and populate the database
for i in range(num_pairs):
    key = generate_random_key()
    value = generate_random_value()
    db.put(key, value)

# Close the database
db.close()

print(f"Generated and populated {num_pairs} key-value pairs in RocksDB at '{db_path}'.")
