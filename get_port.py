"""Basic script to pick up the upload port from a local .env file
"""
import os
try:
    from dotenv import load_dotenv
    load_dotenv()
except ImportError:
    pass

os.environ['UPLOAD_PORT'] = os.getenv('UPLOAD_PORT', '')
