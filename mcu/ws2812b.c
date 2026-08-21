#include "ws2812b.h"

#define WS2812B_RESET_BYTES              16U
#define WS2812B_ENCODED_BYTES_PER_PIXEL   9U
#define WS2812B_SPI_REQUEST_SOURCE      0x21U
#define WS2812B_SPI2BUF_ADDRESS        0x0268U
#define WS2812B_SPI_ZERO_BITS            0x04U
#define WS2812B_SPI_ONE_BITS             0x06U
#define WS2812B_TX_BUFFER_BYTES \
    ((2U * WS2812B_RESET_BYTES) + \
     (WS2812B_ENCODED_BYTES_PER_PIXEL * WS2812B_MAX_PIXELS))

#if configCPU_CLOCK_HZ != 69093750UL
#error "WS2812B SPI encoding requires configCPU_CLOCK_HZ == 69093750UL"
#endif

static u8 ws2812b_tx_buffer[WS2812B_TX_BUFFER_BYTES] __attribute__((aligned(2)));
static u16 ws2812b_rx_discard __attribute__((aligned(2)));
static volatile bool ws2812b_initialized = false;
static volatile bool ws2812b_transfer_busy = false;

static u8 *ws2812b_channel_encode(u8 *output, u8 value)
{
    u32 encoded_value = 0U;
    u8 bit_index;

    for (bit_index = 0U; bit_index < 8U; bit_index++)
    {
        encoded_value <<= 3U;
        encoded_value |= ((value & 0x80U) != 0U)
            ? WS2812B_SPI_ONE_BITS
            : WS2812B_SPI_ZERO_BITS;
        value <<= 1U;
    }

    output[0] = (u8)(encoded_value >> 16U);
    output[1] = (u8)(encoded_value >> 8U);
    output[2] = (u8)encoded_value;

    return output + 3;
}

static u16 ws2812b_frame_encode(const Ws2812bColor *colors, u16 color_count)
{
    u8 *output = ws2812b_tx_buffer;
    u16 byte_index;
    u16 color_index;

    for (byte_index = 0U; byte_index < WS2812B_RESET_BYTES; byte_index++)
    {
        *output++ = 0U;
    }

    for (color_index = 0U; color_index < color_count; color_index++)
    {
        output = ws2812b_channel_encode(output, colors[color_index].green);
        output = ws2812b_channel_encode(output, colors[color_index].red);
        output = ws2812b_channel_encode(output, colors[color_index].blue);
    }

    for (byte_index = 0U; byte_index < WS2812B_RESET_BYTES; byte_index++)
    {
        *output++ = 0U;
    }

    return (u16)((2U * WS2812B_RESET_BYTES) +
        (WS2812B_ENCODED_BYTES_PER_PIXEL * color_count));
}

static bool ws2812b_spi_configuration_valid(void)
{
    return (SPI2STATbits.SPIEN != 0U) &&
        (SPI2CON1bits.MSTEN != 0U) &&
        (SPI2CON1bits.MODE16 == 0U) &&
        (SPI2CON1bits.PPRE == 2U) &&
        (SPI2CON1bits.SPRE == 1U) &&
        (SPI2CON1bits.CKP == 0U) &&
        (SPI2CON1bits.CKE == 0U) &&
        (SPI2CON1bits.DISSDO == 0U) &&
        (SPI2CON2bits.FRMEN == 0U);
}

static void ws2812b_spi_receive_clear(void)
{
    while (SPI2STATbits.SPIRBF != 0U)
    {
        ws2812b_rx_discard = SPI2BUF;
    }

    SPI2STATbits.SPIROV = 0U;
}

static void ws2812b_dma_configure(void)
{
    DMA0CONbits.CHEN = 0U;
    DMA0CONbits.SIZE = 1U;
    DMA0CONbits.DIR = 1U;
    DMA0CONbits.HALF = 0U;
    DMA0CONbits.NULLW = 0U;
    DMA0CONbits.AMODE = 0U;
    DMA0CONbits.MODE = 1U;
    DMA0REQbits.IRQSEL = WS2812B_SPI_REQUEST_SOURCE;
    DMA0REQbits.FORCE = 0U;
    DMA0PAD = WS2812B_SPI2BUF_ADDRESS;
    DMA0STAH = 0U;

    DMA1CONbits.CHEN = 0U;
    DMA1CONbits.SIZE = 1U;
    DMA1CONbits.DIR = 0U;
    DMA1CONbits.HALF = 0U;
    DMA1CONbits.NULLW = 0U;
    DMA1CONbits.AMODE = 1U;
    DMA1CONbits.MODE = 1U;
    DMA1REQbits.IRQSEL = WS2812B_SPI_REQUEST_SOURCE;
    DMA1REQbits.FORCE = 0U;
    DMA1PAD = WS2812B_SPI2BUF_ADDRESS;
    DMA1STAH = 0U;

    _DMA0IE = 0U;
    _DMA0IF = 0U;
    _DMA1IE = 0U;
    _DMA1IF = 0U;
    _DMA1IP = configKERNEL_INTERRUPT_PRIORITY;
    _DMA1IE = 1U;
}

static bool ws2812b_transfer_start(u16 transfer_bytes)
{
    if ((DMA0CONbits.CHEN != 0U) || (DMA1CONbits.CHEN != 0U))
    {
        return false;
    }

    DMA0STAL = __builtin_dmaoffset(ws2812b_tx_buffer);
    DMA0STAH = 0U;
    DMA1STAL = __builtin_dmaoffset(&ws2812b_rx_discard);
    DMA1STAH = 0U;
    DMA0CNT = transfer_bytes - 1U;
    DMA1CNT = transfer_bytes - 1U;

    DMA0REQbits.FORCE = 0U;
    _DMA0IF = 0U;
    _DMA1IF = 0U;
    _SPI2IF = 0U;
    _SPI2EIF = 0U;
    ws2812b_spi_receive_clear();

    DMA1CONbits.CHEN = 1U;
    DMA0CONbits.CHEN = 1U;
    DMA0REQbits.FORCE = 1U;

    return true;
}

bool ws2812b_init(void)
{
    if (ws2812b_initialized || !ws2812b_spi_configuration_valid())
    {
        return false;
    }

    _SPI2IE = 0U;
    _SPI2IF = 0U;
    _SPI2EIE = 0U;
    _SPI2EIF = 0U;
    SPI2STATbits.SPIEN = 0U;
    SPI2CON1bits.DISSCK = 1U;
    SPI2CON2bits.SPIBEN = 0U;
    SPI2STATbits.SISEL = 3U;
    ws2812b_spi_receive_clear();

    ws2812b_dma_configure();

    SPI2STATbits.SPIEN = 1U;
    ws2812b_transfer_busy = false;
    ws2812b_initialized = true;

    return true;
}

bool ws2812b_show(const Ws2812bColor *colors, u16 color_count)
{
    u16 transfer_bytes;

    if (!ws2812b_initialized || (colors == NULL) || (color_count == 0U) ||
        (color_count > WS2812B_MAX_PIXELS))
    {
        return false;
    }

    al_critical_enter();

    if (ws2812b_transfer_busy)
    {
        al_critical_exit();
        return false;
    }

    ws2812b_transfer_busy = true;
    al_critical_exit();

    transfer_bytes = ws2812b_frame_encode(colors, color_count);

    if (!ws2812b_transfer_start(transfer_bytes))
    {
        al_critical_enter();
        ws2812b_transfer_busy = false;
        al_critical_exit();
        return false;
    }

    return true;
}

bool ws2812b_busy(void)
{
    return ws2812b_transfer_busy;
}

void __attribute__((interrupt, no_auto_psv)) _DMA1Interrupt(void)
{
    _DMA1IF = 0U;
    ws2812b_transfer_busy = false;
}
