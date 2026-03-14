import os
import paramiko
import getpass

# --- Server Configuration ---
HOSTNAME = "1ink.us"
PORT = 22
USERNAME = "ford442"

# --- Project Configuration ---
# Only upload the web assets, not build artifacts
LOCAL_DIRECTORY = "build/web"
REMOTE_DIRECTORY = "test.1ink.us/prophecy"

# Files/directories to exclude from upload
EXCLUDE_PATTERNS = [
    '.git',
    '.gitignore',
    '__pycache__',
    '*.pyc',
    '.DS_Store',
    'Thumbs.db',
    '*.log',
    '.env',
    'node_modules',
]

def should_exclude(item_name):
    """Check if item should be excluded based on patterns."""
    import fnmatch
    for pattern in EXCLUDE_PATTERNS:
        if fnmatch.fnmatch(item_name, pattern):
            return True
    return False

def upload_directory(sftp_client, local_path, remote_path):
    """
    Recursively uploads a directory and its contents to the remote server.
    """
    print(f"Creating remote directory: {remote_path}")
    try:
        sftp_client.mkdir(remote_path)
    except IOError:
        print(f"Directory {remote_path} already exists.")

    for item in os.listdir(local_path):
        if should_exclude(item):
            print(f"Skipping excluded item: {item}")
            continue
            
        local_item_path = os.path.join(local_path, item)
        remote_item_path = f"{remote_path}/{item}"

        if os.path.isfile(local_item_path):
            print(f"Uploading file: {local_item_path} -> {remote_item_path}")
            sftp_client.put(local_item_path, remote_item_path)
        elif os.path.isdir(local_item_path):
            upload_directory(sftp_client, local_item_path, remote_item_path)

def main():
    """
    Main function to connect to the server and start the upload process.
    """
    password = 'GoogleBez12!'

    transport = None
    sftp = None
    try:
        transport = paramiko.Transport((HOSTNAME, PORT))
        print("Connecting to server...")
        transport.connect(username=USERNAME, password=password)
        print("Connection successful!")

        sftp = paramiko.SFTPClient.from_transport(transport)
        print(f"Starting upload of '{LOCAL_DIRECTORY}' to '{REMOTE_DIRECTORY}'...")

        upload_directory(sftp, LOCAL_DIRECTORY, REMOTE_DIRECTORY)

        print("\n✅ Deployment complete!")

    except Exception as e:
        print(f"❌ An error occurred: {e}")
    finally:
        if sftp:
            sftp.close()
        if transport:
            transport.close()
        print("Connection closed.")

if __name__ == "__main__":
    if not os.path.exists(LOCAL_DIRECTORY):
        print(f"Error: Local directory '{LOCAL_DIRECTORY}' not found.")
        print("Did you run './build.sh' first?")
    else:
        main()
