"""Basic script to pick up the upload port from a local .env file
"""
import os
try:
    from dotenv import load_dotenv
    load_dotenv()
except ImportError:
    pass

os.environ['PLATFORMIO_UPLOAD_PORT'] = os.getenv('PLATFORMIO_UPLOAD_PORT', '')
