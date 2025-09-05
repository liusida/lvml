# Mount VFS filesystem
import vfs
from esp32 import Partition
partitions = Partition.find(Partition.TYPE_DATA)
target_partition = None

for p in partitions:
    info = p.info()
    if len(info) >= 5 and info[4] == "vfs":
        target_partition = p
        break
if target_partition:
    vfs.mount(target_partition, "/")

# Initialize LVML and show splash screen
import lvml
import png_lvml
lvml.init()
lvml.show_image(png_lvml.PNG_DATA)
lvml.tick()

# Connect to WiFi
import network
import esp32
wifi = network.WLAN(network.STA_IF)
wifi.active(True)
nvs = esp32.NVS("lvml")
try:
    buf = bytearray(256)
    size = nvs.get_blob("wifi_ssid", buf)
    wifi_ssid = buf[:size].decode("utf-8")
    size = nvs.get_blob("wifi_password", buf)
    wifi_password = buf[:size].decode("utf-8")
    if wifi_ssid and wifi_password:
        wifi.connect(wifi_ssid, wifi_password)
except Exception as e:
    print(e)
    print("No WiFi settings found")
    
    # Load WiFi settings UI
    try:
        with open("/web/wifi_settings.xml", "r") as f:
            xml_content = f.read()
        lvml.load_xml(xml_content)
        lvml.tick()
        print("WiFi settings UI loaded")
    except Exception as ui_error:
        print("Failed to load WiFi settings UI:", ui_error)
