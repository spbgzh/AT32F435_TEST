#include "psram.h"
#include "at32f435_437.h"

qspi_cmd_type ly68l6400_cmd_config;
qspi_xip_type ly68l6400_xip_init;

void ly68l6400_init(void)
{
    gpio_init_type gpio_init_struct;

    /* enable the qspi clock */
    crm_periph_clock_enable(CRM_QSPI2_PERIPH_CLOCK, TRUE);

    /* enable the pin clock */
    crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);

    /* set default parameter */
    gpio_default_para_init(&gpio_init_struct);

    /* configure the io0 gpio */
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pins = GPIO_PINS_0;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init(GPIOB, &gpio_init_struct);
    gpio_pin_mux_config(GPIOB, GPIO_PINS_SOURCE0, GPIO_MUX_9);

    /* configure the io1 gpio */
    gpio_init_struct.gpio_pins = GPIO_PINS_7;
    gpio_init(GPIOB, &gpio_init_struct);
    gpio_pin_mux_config(GPIOB, GPIO_PINS_SOURCE7, GPIO_MUX_9);

    /* configure the io2 gpio */
    gpio_init_struct.gpio_pins = GPIO_PINS_5;
    gpio_init(GPIOA, &gpio_init_struct);
    gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE5, GPIO_MUX_9);

    /* configure the io3 gpio */
    gpio_init_struct.gpio_pins = GPIO_PINS_3;
    gpio_init(GPIOA, &gpio_init_struct);
    gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE3, GPIO_MUX_9);

    /* configure the sck gpio */
    gpio_init_struct.gpio_pins = GPIO_PINS_1;
    gpio_init(GPIOB, &gpio_init_struct);
    gpio_pin_mux_config(GPIOB, GPIO_PINS_SOURCE1, GPIO_MUX_10);

    /* configure the cs gpio */
    gpio_init_struct.gpio_pins = GPIO_PINS_8;
    gpio_init(GPIOB, &gpio_init_struct);
    gpio_pin_mux_config(GPIOB, GPIO_PINS_SOURCE8, GPIO_MUX_10);

    /* switch to cmd port */
    qspi_xip_enable(QSPI2, FALSE);

    /* set sclk */
    qspi_clk_division_set(QSPI2, QSPI_CLK_DIV_4);

    /* set sck idle mode 0 */
    qspi_sck_mode_set(QSPI2, QSPI_SCK_MODE_0);

    /* set wip in bit 0 */
    qspi_busy_config(QSPI2, QSPI_BUSY_OFFSET_0);
}

void ly68l6400_cmd_rsten_config(qspi_cmd_type *qspi_cmd_struct)
{
    qspi_cmd_struct->pe_mode_enable = FALSE;
    qspi_cmd_struct->pe_mode_operate_code = 0;
    qspi_cmd_struct->instruction_code = 0x66;
    qspi_cmd_struct->instruction_length = QSPI_CMD_INSLEN_1_BYTE;
    qspi_cmd_struct->address_code = 0;
    qspi_cmd_struct->address_length = QSPI_CMD_ADRLEN_0_BYTE;
    qspi_cmd_struct->data_counter = 0;
    qspi_cmd_struct->second_dummy_cycle_num = 0;
    qspi_cmd_struct->operation_mode = QSPI_OPERATE_MODE_111;
    qspi_cmd_struct->read_status_config = QSPI_RSTSC_HW_AUTO;
    qspi_cmd_struct->read_status_enable = FALSE;
    qspi_cmd_struct->write_data_enable = TRUE;
}

void ly68l6400_xip_init_config(qspi_xip_type *qspi_xip_struct)
{
    qspi_xip_struct->read_instruction_code = 0xEB;
    qspi_xip_struct->read_address_length = QSPI_XIP_ADDRLEN_3_BYTE;
    qspi_xip_struct->read_operation_mode = QSPI_OPERATE_MODE_144;
    qspi_xip_struct->read_second_dummy_cycle_num = 6;
    qspi_xip_struct->write_instruction_code = 0x38;
    qspi_xip_struct->write_address_length = QSPI_XIP_ADDRLEN_3_BYTE;
    qspi_xip_struct->write_operation_mode = QSPI_OPERATE_MODE_144;
    qspi_xip_struct->write_second_dummy_cycle_num = 0;
    qspi_xip_struct->write_select_mode = QSPI_XIPW_SEL_MODED;
    qspi_xip_struct->write_time_counter = 0x7F;
    qspi_xip_struct->write_data_counter = 0x1F;
    qspi_xip_struct->read_select_mode = QSPI_XIPR_SEL_MODED;
    qspi_xip_struct->read_time_counter = 0x7F;
    qspi_xip_struct->read_data_counter = 0x1F;
}

void ly68l6400_cmd_rst_config(qspi_cmd_type *qspi_cmd_struct)
{
    qspi_cmd_struct->pe_mode_enable = FALSE;
    qspi_cmd_struct->pe_mode_operate_code = 0;
    qspi_cmd_struct->instruction_code = 0x99;
    qspi_cmd_struct->instruction_length = QSPI_CMD_INSLEN_1_BYTE;
    qspi_cmd_struct->address_code = 0;
    qspi_cmd_struct->address_length = QSPI_CMD_ADRLEN_0_BYTE;
    qspi_cmd_struct->data_counter = 0;
    qspi_cmd_struct->second_dummy_cycle_num = 0;
    qspi_cmd_struct->operation_mode = QSPI_OPERATE_MODE_111;
    qspi_cmd_struct->read_status_config = QSPI_RSTSC_HW_AUTO;
    qspi_cmd_struct->read_status_enable = FALSE;
    qspi_cmd_struct->write_data_enable = TRUE;
}

void ly68l6400_xip_mode_init(void)
{
    /* switch to command-port mode */
    qspi_xip_enable(QSPI2, FALSE);

    /* issue reset command */
    ly68l6400_cmd_rsten_config(&ly68l6400_cmd_config);
    qspi_cmd_operation_kick(QSPI2, &ly68l6400_cmd_config);
    /* wait command completed */
    while (qspi_flag_get(QSPI2, QSPI_CMDSTS_FLAG) == RESET)
        ;
    qspi_flag_clear(QSPI2, QSPI_CMDSTS_FLAG);
    ly68l6400_cmd_rst_config(&ly68l6400_cmd_config);
    qspi_cmd_operation_kick(QSPI2, &ly68l6400_cmd_config);
    /* wait command completed */
    while (qspi_flag_get(QSPI2, QSPI_CMDSTS_FLAG) == RESET)
        ;
    qspi_flag_clear(QSPI2, QSPI_CMDSTS_FLAG);

    /* initial xip */
    ly68l6400_xip_init_config(&ly68l6400_xip_init);
    qspi_xip_init(QSPI2, &ly68l6400_xip_init);
    qspi_xip_enable(QSPI2, TRUE);
}

void ly68l6400_command_mode_init(void)
{
    /* 禁用QSPI的XIP模式 */
    qspi_xip_enable(QSPI2, FALSE);

    /* issue reset command */
    ly68l6400_cmd_rsten_config(&ly68l6400_cmd_config);
    qspi_cmd_operation_kick(QSPI2, &ly68l6400_cmd_config);
    /* wait command completed */
    while (qspi_flag_get(QSPI2, QSPI_CMDSTS_FLAG) == RESET)
        ;
    qspi_flag_clear(QSPI2, QSPI_CMDSTS_FLAG);
    ly68l6400_cmd_rst_config(&ly68l6400_cmd_config);
    qspi_cmd_operation_kick(QSPI2, &ly68l6400_cmd_config);
    /* wait command completed */
    while (qspi_flag_get(QSPI2, QSPI_CMDSTS_FLAG) == RESET)
        ;
    qspi_flag_clear(QSPI2, QSPI_CMDSTS_FLAG);
}

uint16_t ly68l6400_readID(void)
{
    uint16_t temp = 0, ID = 0;

    qspi_cmd_type qspi_cmd_struct;

    qspi_cmd_struct.pe_mode_enable = FALSE;
    qspi_cmd_struct.pe_mode_operate_code = 0;
    qspi_cmd_struct.instruction_code = 0x9F;
    qspi_cmd_struct.instruction_length = QSPI_CMD_INSLEN_1_BYTE;
    qspi_cmd_struct.address_code = 0;
    qspi_cmd_struct.address_length = QSPI_CMD_ADRLEN_3_BYTE;
    qspi_cmd_struct.data_counter = 2;
    qspi_cmd_struct.second_dummy_cycle_num = 0;
    qspi_cmd_struct.operation_mode = QSPI_OPERATE_MODE_111;
    qspi_cmd_struct.read_status_config = QSPI_RSTSC_HW_AUTO;
    qspi_cmd_struct.read_status_enable = FALSE;
    qspi_cmd_struct.write_data_enable = FALSE;
    qspi_cmd_operation_kick(QSPI2, &qspi_cmd_struct);
    while (qspi_flag_get(QSPI2, QSPI_RXFIFORDY_FLAG) == RESET)
        ;

    temp = qspi_half_word_read(QSPI2);

    while (qspi_flag_get(QSPI2, QSPI_CMDSTS_FLAG) == RESET)
        ;
    qspi_flag_clear(QSPI2, QSPI_CMDSTS_FLAG);

    ID = (temp << 8) | (temp >> 8);

    return ID;
}
