"""
Simple Calculator Compiler - Main Runner
----------------------------------------
Starts the compiler web server and opens frontend.html in the default browser.
"""

import sys
import webbrowser
import threading
import time
from calculator import start_server


def main():
    port = 5000
    if len(sys.argv) > 1:
        try:
            port = int(sys.argv[1])
        except ValueError:
            pass

    print("=" * 60)
    print("⚡ Starting Simple Calculator Compiler Studio...")
    print(f"📡 Server URL: http://localhost:{port}")
    print("=" * 60)

    # Launch browser after a brief moment
    def launch_browser():
        time.sleep(0.8)
        webbrowser.open(f"http://localhost:{port}")

    threading.Thread(target=launch_browser, daemon=True).start()

    # Start the HTTP server
    start_server(port=port, open_browser=False)


if __name__ == "__main__":
    main()
