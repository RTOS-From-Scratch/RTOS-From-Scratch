#include "Drivers/src/driver.h"
#include "Drivers/src/GPIO.h"
#include "Drivers/src/ADC.h"
#include "Drivers/src/UART.h"
#include "Drivers/src/time.h"
#include "FileSystem/src/ff.h"
#include "Kernel/src/nanokernel/nanokernel.h"
#include "Drivers/src/LCD/LCD.h"
#include "DataStructures/src/mailbox.h"

FIL fp;

struct LCD_Mailboxes
{
    Mailbox *photoResistorMB;
    Mailbox *temperatureMB;
};

void temperatureSensorTask(void* param)
{

    while(true)
    {

    }
}

void photoResistorSensorTask(void* param)
{
    Mailbox* mailbox = (Mailbox *) param;
    ADC_Driver* adc = nanokernel_Task_requestDriver(ADC, ADC6);
    ADC_init(adc, ADC_Sample_Rate_125KsPs, true);
    int adc_readData;

    while(1)
    {
        adc_readData = ADC_read(adc);
        Mailbox_request_write( mailbox, &adc_readData );
    }
}

void LCDTask(void* param)
{
    int buffer;
    struct LCD_Mailboxes* mailboxes = (struct LCD_Mailboxes*)param;

    while(1)
    {
        buffer = *(int *)Mailbox_request_read(mailboxes->photoResistorMB);
        LCD_print( "temp" );
        LCD_writeInt(buffer);
        SYS_UART_writeInt(buffer);
        SYS_UART_println("");
    }
}

void MICTask(void* param)
{

}

void serialCommHandler()
{

}

void serialComm_SendPushBtnHandler()
{

}

void serialComm_SelectPushBtnHandler()
{

}

int main()
{
    nanokernel_init(TASKLESS);

/********************** Serial communication protocol ***********************/
    UART_Driver* serialComm = nanokernel_Task_requestDriver( UART, U1_PORTB );
    UART_ISR_init( serialComm,
                   UART_BAUDRATE_115200,
                   UART_ISR_MODE_Rx,
                   ISR_PRIORITY_1,
                   serialCommHandler,
                   true );
    GPIO_Driver* serialComm_SendPushBtn = nanokernel_Task_requestDriver( GPIO, F4 );
    GPIO_ISR_init( serialComm_SendPushBtn,
                   INPUT_PULLUP,
                   GPIO_ISR_MODE_Edge_FALLING,
                   ISR_PRIORITY_1,
                   serialComm_SendPushBtnHandler,
                   true );
    GPIO_Driver* serialComm_SelectPushBtn = nanokernel_Task_requestDriver( GPIO, F0 );
    GPIO_ISR_init( serialComm_SelectPushBtn,
                   INPUT_PULLUP,
                   GPIO_ISR_MODE_Edge_FALLING,
                   ISR_PRIORITY_1,
                   serialComm_SelectPushBtnHandler,
                   true );
    GPIO_Driver* serialComm_msgRedLed = nanokernel_Task_requestDriver( GPIO, F1 );
    GPIO_init( serialComm_msgRedLed, PIN_MODE_OUTPUT, true );
    GPIO_Driver* serialComm_msgBlueLed = nanokernel_Task_requestDriver( GPIO, F2 );
    GPIO_init( serialComm_msgBlueLed, PIN_MODE_OUTPUT, true );
    GPIO_Driver* serialComm_msgGreenLed = nanokernel_Task_requestDriver( GPIO, F3 );
    GPIO_init( serialComm_msgGreenLed, PIN_MODE_OUTPUT, true );
/*****************************************************************************/

/****************************** LCD initiation ******************************/
    LCD_init4BitsWithoutRW( D6, D7, C4, C5, C6, C7 );
/****************************************************************************/

/*************************** Mailboxes initiation ***************************/
    Mailbox *photoResistorMB = Mailbox_create( sizeof(int) );
    Mailbox *temperatureMB = Mailbox_create( sizeof(int) );

    struct LCD_Mailboxes *LCD_mailboxes = malloc( sizeof(struct LCD_Mailboxes) );
    LCD_mailboxes->photoResistorMB = photoResistorMB;
    LCD_mailboxes->temperatureMB = temperatureMB;
/****************************************************************************/

/*********************************** Tasks ***********************************/
    nanokernel_Task_create( 200, PRIORITY_2, temperatureSensorTask, NULL, 1 );
    nanokernel_Task_create( 200, PRIORITY_2, photoResistorSensorTask, photoResistorMB, 1 );
    nanokernel_Task_create( 200, PRIORITY_2, LCDTask, LCD_mailboxes, 0 );
    // nanokernel_Task_create( 200, PRIORITY_2, MICTask, NULL, 1 );

    // nanokernel_Task_create( 200, PRIORITY_2, serialCommunicationTask, NULL, 1 );
    // nanokernel_Task_create( 200, PRIORITY_2, temperatureSensorTask, NULL, 2 );
/****************************************************************************/

    FATFS* fs = malloc(sizeof(FATFS));
    //
    f_mount( fs, "", 1 );
        if(f_open(&fp, "sara.txt", FA_WRITE | FA_OPEN_APPEND) is FR_OK)
        {
            if(f_write(&fp, buf, 15, NULL) is FR_OK)
//            if( f_read(&fp, buf, 15, NULL) is FR_OK )
            {
                f_sync(&fp);
                GPIO_write(serialComm_msgGreenLed, HIGH);
                SYS_UART_println(buf);
            }
        }

        f_mount( NULL, "", 1 );
    }

    nanokernel_bootup();
}
