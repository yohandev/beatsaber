from machine import Pin, I2C
from utime import sleep, ticks_ms
from struct import pack
from sys import stdout
from math import pi

# from nrf24l01 import NRF24L01
from mpu6050 import MPU6050

# Pin configuration
cfg = {
    "spi": 0, "miso": 9, "mosi": 12, "sck": 6, "csn": 14, "ce": 17,
    "i2c": 0, "sda": 4, "scl": 5
}

# csn = Pin(cfg["csn"], mode=Pin.OUT)
# ce  = Pin(cfg["ce"], mode=Pin.OUT)
# spi = SPI(cfg["spi"])
# nrf = NRF24L01(spi, csn, ce, payload_size=8)

i2c = I2C(cfg["i2c"], sda=Pin(cfg["sda"]), scl=Pin(cfg["scl"]), freq=400_000)
imu = MPU6050(i2c)

rot = (0, 0, 0)
t = ticks_ms()

while True:
    # Delta time
    dt = (ticks_ms() - t) / 1000
    t = ticks_ms()

    # Integrate
    rot = [p + n * dt for (p, n) in zip(rot, imu.gyro.xyz)] # type: ignore

    # Notify
    stdout.write(pack("fff", *[n * (pi / 180) for n in rot]))
    print()
    sleep(0.02)