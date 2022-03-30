import subprocess

if __name__ == "__main__":
    subprocess.call([
        'python',
        'run-clang-format.py', '-r', 'src', 'include', 'testWnd/src', 'testWnd/include'
    ])
    subprocess.call([
        'python',
        'run-clang-format.py', '-r', '-i', 'src', 'include', 'testWnd/src', 'testWnd/include'
    ])
