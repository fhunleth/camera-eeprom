#include <QCoreApplication>
#include <QDataStream>
#include <QFile>

#include <QDate>

static void writeRawString(QDataStream &ds, const char *str, int outputLen)
{
    char output[outputLen];
    memset(output, 0, outputLen);
    strncpy(output, str, outputLen);

    ds.writeRawData(output, outputLen);
}

#define USED        (1 << 15)
#define UNUSED      (0 << 15)

#define INPUT       ((1 << 13) | (1 << 5))  /* Type=input, RX=1 */
#define OUTPUT      (2 << 13)
#define BIDIR       (3 << 13)

#define PULL_UP     (1 << 4)
#define PULL_DN     (0 << 4)

#define NO_PULLUPDN (1 << 3)

#define SLEWCTRL_SLOW (1 << 6)

#define MUXMODE(x)  (x & 7)

#define CONFIGURABLE_PIN_COUNT (74)
static quint16 pinUsage[CONFIGURABLE_PIN_COUNT] = {
    /* P9-22 UART2_RXD */  USED | BIDIR | MUXMODE (1),
    /* P9-21 UART2_TXD */  USED | BIDIR | MUXMODE (1),
    /* P9-18 I2C1_SDA */   USED | INPUT | SLEWCTRL_SLOW | PULL_UP | MUXMODE (2), // Match DTS Pinmux config
    /* P9-17 I2C1_SCL */   USED | INPUT | SLEWCTRL_SLOW | PULL_UP | MUXMODE (2), // Match DTS Pinmux config
    /* P9-42 GPIO0_7 */    UNUSED,
    /* P8-35 UART4_CTSN */ UNUSED,
    /* P8-33 UART4_RTSN */ UNUSED,
    /* P8-31 UART5_CTSN */ UNUSED,
    /* P8-32 UART5_RTSN */ UNUSED,
    /* P9-19 I2C2_SCL */   USED | OUTPUT | MUXMODE (3),
    /* P9-20 I2C2_SDA */   USED | BIDIR | MUXMODE (3),
    /* P9-26 UART1_RXD */  USED | INPUT | NO_PULLUPDN | MUXMODE (6),
    /* P9-24 UART1_TXD */  USED | INPUT | NO_PULLUPDN | MUXMODE (6),
    /* P9-41 CLKOUT2 */    UNUSED,
    /* P8-19 EHRPWM2A */   USED | OUTPUT | MUXMODE (4),
    /* P8-13 EHRPWM2B */   UNUSED,
    /* P8-14 GPIO0_26 */   UNUSED,
    /* P8-17 GPIO0_27 */   UNUSED,
    /* P9-11 UART4_RXD */  USED | OUTPUT| MUXMODE (7),
    /* P9-13 UART4_TXD */  USED | OUTPUT| MUXMODE (7),
    /* P8-25 GPIO1_0 */    UNUSED,
    /* P8-24 GPIO1_1 */    UNUSED,
    /* P8-5 GPIO1_2 */     UNUSED,
    /* P8-6 GPIO1_3 */     UNUSED,
    /* P8-23 GPIO1_4 */    UNUSED,
    /* P8-22 GPIO1_5 */    UNUSED,
    /* P8-3 GPIO1_6 */     UNUSED,
    /* P8-4 GPIO1_7 */     UNUSED,
    /* P8-12 GPIO1_12 */   UNUSED,
    /* P8-11 GPIO1_13 */   UNUSED,
    /* P8-16 GPIO1_14 */   UNUSED,
    /* P8-15 GPIO1_15 */   UNUSED,
    /* P9-15 GPIO1_16 */   USED | OUTPUT | MUXMODE(7),
    /* P9-23 GPIO1_17 */   UNUSED,
    /* P9-14 EHRPWM1A */   UNUSED,
    /* P9-16 EHRPWM1B */   UNUSED,
    /* P9-12 GPIO1_18 */   USED | INPUT | NO_PULLUPDN | MUXMODE(7),
    /* P8-26 GPIO1_29 */   UNUSED,
    /* P8-21 GPIO1_30 */   UNUSED,
    /* P8-20 GPIO1_31 */   UNUSED,
    /* P8-18 GPIO2_1 */    UNUSED,
    /* P8-7 TIMER4 */      UNUSED,
    /* P8-9 TIMER5 */      UNUSED,
    /* P8-10 TIMER6 */     UNUSED,
    /* P8-8 TIMER7 */      UNUSED,
    /* P8-45 GPIO2_6 */    USED | INPUT | NO_PULLUPDN | MUXMODE(6),
    /* P8-46 GPIO2_7 */    USED | INPUT | NO_PULLUPDN | MUXMODE(6),
    /* P8-43 GPIO2_8 */    USED | INPUT | NO_PULLUPDN | MUXMODE(6),
    /* P8-44 GPIO2_9 */    USED | INPUT | NO_PULLUPDN | MUXMODE(6),
    /* P8-41 GPIO2_10 */   USED | INPUT | NO_PULLUPDN | MUXMODE(6),
    /* P8-42 GPIO2_11 */   USED | INPUT | NO_PULLUPDN | MUXMODE(6),
    /* P8-39 GPIO2_12 */   USED | INPUT | NO_PULLUPDN | MUXMODE(6),
    /* P8-40 GPIO2_13 */   USED | INPUT | NO_PULLUPDN | MUXMODE(6),
    /* P8-37 UART5_TXD */  UNUSED,
    /* P8-38 UART5_RXD */  UNUSED,
    /* P8-36 UART3_CTSN */ UNUSED,
    /* P8-34 UART3_RTSN */ UNUSED,
    /* P8-27 GPIO2_22 */   USED | INPUT | NO_PULLUPDN | MUXMODE(6),
    /* P8-29 GPIO2_23 */   USED | INPUT | NO_PULLUPDN | MUXMODE(6),
    /* P8-28 GPIO2_24 */   USED | INPUT | NO_PULLUPDN | MUXMODE(6),
    /* P8-30 GPIO2_25 */   USED | INPUT | NO_PULLUPDN | MUXMODE(6),
    /* P9-29 SPI1_D0 */    UNUSED,
    /* P9-30 SPI1_D1 */    UNUSED,
    /* P9-28 SPI1_CS0 */   UNUSED,
    /* P9-27 GPIO3_19 */   USED | INPUT | NO_PULLUPDN | MUXMODE(7),
    /* P9-31 SPI1_SCLK */  UNUSED,
    /* P9-25 GPIO3_21 */   USED | MUXMODE(5)
};

static int usedPins()
{
    int total = 0;
    for (int i = 0; i < CONFIGURABLE_PIN_COUNT; i++)
        if (pinUsage[i] & USED)
            total++;

    return total + 8 /* grounds */ + 2 /* DC 3.3V */;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("%s <outputfile.bin>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    QFile fp(argv[1]);
    if (!fp.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        printf("Couldn't open '%s'.\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    QDataStream ds(&fp);
    ds.setByteOrder(QDataStream::BigEndian);

    // Header
    Q_ASSERT(fp.pos() == 0);
    writeRawString(ds, "\xaa\x55\x33\xee", 4);

    // EEPROM Revision
    Q_ASSERT(fp.pos() == 4);
    writeRawString(ds, "A1", 2);

    // Board name
    Q_ASSERT(fp.pos() == 6);
    writeRawString(ds, "cape-bone-troodon-cam", 32);

    // Version
    Q_ASSERT(fp.pos() == 38);
    writeRawString(ds, "00A0", 4);

    // Manufacturer
    Q_ASSERT(fp.pos() == 42);
    writeRawString(ds, "Troodon Software", 16);

    // Part Number
    Q_ASSERT(fp.pos() == 58);
    writeRawString(ds, "troodon-cam", 16);

    // Number of pins used
    Q_ASSERT(fp.pos() == 74);
    ds << quint16(usedPins());

    // Serial Number
    Q_ASSERT(fp.pos() == 76);
    QDate now = QDate::currentDate();
    QString serialNumber = QString("%1%2%3%4")
            .arg(now.weekNumber(), 2, 10, QLatin1Char('0'))
            .arg(now.year() % 100, 2, 10, QLatin1Char('0'))
            .arg(0, 4, 10, QLatin1Char('0')) // Assembly code
            .arg(1, 4, 10, QLatin1Char('0')); // Board #
    writeRawString(ds, serialNumber.toLatin1().data(), 12);

    // Pin Usage
    Q_ASSERT(fp.pos() == 88);
    for (int i = 0; i < CONFIGURABLE_PIN_COUNT; i++)
        ds << pinUsage[i];

    // VDD_3V3B Current
    Q_ASSERT(fp.pos() == 236);
    ds << quint16(100); // 100 mA?

    // VDD_5V Current
    Q_ASSERT(fp.pos() == 238);
    ds << quint16(0); // Unused

    // SYS_5V Current
    Q_ASSERT(fp.pos() == 240);
    ds << quint16(0); // Unused

    // DC Supplied
    Q_ASSERT(fp.pos() == 242);
    ds << quint16(0); // Not supplied.

    fp.close();
    exit(EXIT_SUCCESS);
}
