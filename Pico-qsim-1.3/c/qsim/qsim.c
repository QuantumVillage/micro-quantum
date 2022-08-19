#include "qsim.h"
//#include "LCD_1in14.h"
#include "LCD_1in3.h"
#include "gates.h"
#include "measure.h"
#include "img_hex.h"
#include <string.h>
#include <math.h>

char bin_strs[16][5] = {
    "0000",
    "0001",
    "0010",
    "0011",
    "0100",
    "0101",
    "0110",
    "0111",
    "1000",
    "1001",
    "1010",
    "1011",
    "1100",
    "1101",
    "1110",
    "1111",
};

char help_gates[10][65] = {
    "- : Identity",
    "H: Hadamard",
    "X: NOT gate",
    "R: fixed Rx(-pi/4)",
    "V: sqrt(NOT) gate",
    "c: ctrl operator",
    "x: SWAP gates",
    "All gates can have",
    "c added and x gates",
    "must be paired"
};


/* set address */
bool reserved_addr(uint8_t addr) {
return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

void draw_help_gates(uint16_t* BlackImage) {
    Paint_Clear(WHITE);

    char buff_instr[64];
    for (int i = 0; i < 10; i++){
        sprintf(buff_instr, help_gates[i]);
        Paint_DrawString_EN(0,i*20,buff_instr,&Font16, WHITE, BLACK);
    }
    LCD_1IN3_Display(BlackImage);
    
    // wait for button presses
    uint8_t keyA = 15; 
    uint8_t keyB = 17;
    uint8_t keyX = 19;
    uint8_t keyY = 21;
    while(1)
    {
        if(DEV_Digital_Read(keyA) == 0){
            return;
        }
        if(DEV_Digital_Read(keyB) == 0){
            return;
        }
        if(DEV_Digital_Read(keyX) == 0){
            return;
        }
        if(DEV_Digital_Read(keyY) == 0){
            return;
        }
    }
}

void draw_circuit(char circuit[14][4][8], uint16_t* BlackImage)
{
    Paint_Clear(WHITE);
    for (int i = 0; i < 14; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            Paint_DrawString_EN(i*16, j*20, circuit[i][j], &Font20, WHITE, BLACK);
        }
    }
    char buff_instr[64];
    sprintf(buff_instr, "Move around: U/D/L/R");
    Paint_DrawString_EN(0,90,buff_instr,&Font16, WHITE, BLACK);
    sprintf(buff_instr, "Change gate: push");
    Paint_DrawString_EN(0,110,buff_instr,&Font16, WHITE, BLACK);
    sprintf(buff_instr, "Run circuit: A");
    Paint_DrawString_EN(0,130,buff_instr,&Font16, WHITE, BLACK);
    sprintf(buff_instr, "Clear circuit: B");
    Paint_DrawString_EN(0,150,buff_instr,&Font16, WHITE, BLACK);
    sprintf(buff_instr, "Gate Details: X");
    Paint_DrawString_EN(0,170,buff_instr,&Font16, WHITE, BLACK);
    //LCD_1IN14_Display(BlackImage);
    LCD_1IN3_Display(BlackImage);
}

void init_circuit(char circuit[14][4][8])
{
    for (int i = 0; i < 14; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            strcpy(circuit[i][j], "-");
        }
    }
}

void next_gate(char* in_gate)
{
    if(strcmp(in_gate, "-") == 0){
        strcpy(in_gate, "X");
    }
    else if(strcmp(in_gate, "X") == 0){
        strcpy(in_gate, "H");
    }
    else if(strcmp(in_gate, "H") == 0){
        strcpy(in_gate, "R");
    }
    else if(strcmp(in_gate, "R") == 0){
        strcpy(in_gate, "V");
    }
    else if(strcmp(in_gate, "V") == 0){
        strcpy(in_gate, "c");
    }
    else if(strcmp(in_gate, "c") == 0){
        strcpy(in_gate, "x");
    }
    else if(strcmp(in_gate, "x") == 0){
        strcpy(in_gate, "-");
    }
    else {
        strcpy(in_gate, "A");
    }
}

void draw_results(double complex stateVec[16], UWORD* BlackImage)
{
    Paint_Clear(WHITE);
    int ctr_x = 0;
    int ctr_y = 1;
    char buff[64];
    sprintf(buff, "Output Statevector:");
    Paint_DrawString_EN(0,0,buff,&Font16, WHITE, BLACK);
    for (int i = 0; i < 16; i++)
    {
        if (stateVec[i] != 0)
        {
            char buff[64];
            sprintf(buff, " %s: %.4f ", bin_strs[i], pow(cabs(stateVec[i]),2));
            //sprintf(buff, "%s: %.3f", bin_strs[i], creal(stateVec[i]));
            Paint_DrawString_EN(ctr_x*120, ctr_y*14, buff, &Font16, BLACK, WHITE);
            ++ctr_y;
            //if (ctr_y > 0 && ctr_y % 8 == 0) // for when we go to 5+ qubits... -MC
            //{
            //    // move to next column
            //    ++ctr_x;
            //    ctr_y = 0;
            //}
        }
    }
    //LCD_1IN14_Display(BlackImage);
    LCD_1IN3_Display(BlackImage);

    /*
    char buff[80];
    sprintf(buff, "%.5f", creal(stateVec[0]));
    Paint_DrawString_EN(1, 100, buff, &Font12, WHITE, BLACK);
    LCD_1IN14_Display(BlackImage);
    */

    // wait for button presses
    uint8_t keyA = 15; 
    uint8_t keyB = 17;
    uint8_t keyX = 19;
    uint8_t keyY = 21;
    while(1)
    {
        if(DEV_Digital_Read(keyA) == 0){
            return;
        }
        if(DEV_Digital_Read(keyB) == 0){
            return;
        }
        if(DEV_Digital_Read(keyX) == 0){
            return;
        }
        if(DEV_Digital_Read(keyY) == 0){
            return;
        }

    }
    return;
}

int qsim(void)
{
    DEV_Delay_ms(100);
    //printf("LCD_1in3_test Demo\r\n");
    if(DEV_Module_Init()!=0){
        return -1;
    }
    DEV_SET_PWM(50);
    /* LCD Init */
    LCD_1IN3_Init(HORIZONTAL);
    LCD_1IN3_Clear(WHITE);
    
    //LCD_SetBacklight(1023);
    //UDOUBLE Imagesize = LCD_1IN14_HEIGHT*LCD_1IN14_WIDTH*2;
    UDOUBLE Imagesize = LCD_1IN3_HEIGHT*LCD_1IN3_WIDTH*2;
    UWORD *BlackImage;
    if((BlackImage = (UWORD *)malloc(Imagesize)) == NULL) {
        printf("Failed to apply for black memory...\r\n");
        exit(0);
    }
    // /*1.Create a new image cache named IMAGE_RGB and fill it with white*/
    //Paint_NewImage((UBYTE *)BlackImage,LCD_1IN14.WIDTH,LCD_1IN14.HEIGHT, 0, WHITE);
    Paint_NewImage((UBYTE *)BlackImage,LCD_1IN3.WIDTH,LCD_1IN3.HEIGHT, 0, WHITE);
    Paint_SetScale(65);
    Paint_Clear(WHITE);
    Paint_SetRotate(ROTATE_0);
    Paint_Clear(WHITE);

    Paint_DrawImage(img_buf,0,0,240,240);
    LCD_1IN3_Display(BlackImage);
    DEV_Delay_ms(2000);

    //Paint_DrawCircle(130, 20, 15, GREEN, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    //Paint_DrawNum (50, 40 ,9.87654321, &Font20,3,  WHITE,  BLACK);
    Paint_DrawString_EN(1, 1, "Welcome to", &Font20, WHITE, BLACK);
    Paint_DrawString_EN(20, 20, "QSim v0.1", &Font20, WHITE, BLACK);

    //LCD_1IN14_Display(BlackImage);
    LCD_1IN3_Display(BlackImage);
    DEV_Delay_ms(2000);
    DEV_SET_PWM(20); // dim the lights

    uint8_t keyA = 15; 
    uint8_t keyB = 17; 
    uint8_t keyX = 19;
    uint8_t keyY = 21;

    uint8_t up = 2;
	uint8_t dowm = 18;
	uint8_t left = 16;
	uint8_t right = 20;
	uint8_t ctrl = 3;
   

    SET_Infrared_PIN(keyA);    
    SET_Infrared_PIN(keyB);
    SET_Infrared_PIN(keyX);
    SET_Infrared_PIN(keyY);
		 
	SET_Infrared_PIN(up);
    SET_Infrared_PIN(dowm);
    SET_Infrared_PIN(left);
    SET_Infrared_PIN(right);
    SET_Infrared_PIN(ctrl);
    
    Paint_Clear(WHITE);

    // setup cursor variables
    int cursor_x = 0;
    int cursor_y = 0;

    // init circuit and state
    double complex stateVec[16] = {0};
    stateVec[0] = 1 + 0 * I;
    char circuit[14][4][8];

    // init circuit
    init_circuit(circuit);

    // setup entanglement circuit
    //strcpy(circuit[0][0], "H");
    //strcpy(circuit[1][0], "c");
    //strcpy(circuit[1][1], "X");

    // setup entanglement circuit
    strcpy(circuit[0][0], "H");
    strcpy(circuit[0][2], "H");
    strcpy(circuit[0][3], "H");
    strcpy(circuit[1][0], "c");
    strcpy(circuit[1][1], "X");
    strcpy(circuit[2][0], "R");
    strcpy(circuit[3][0], "V");
    strcpy(circuit[3][2], "c");
    strcpy(circuit[4][0], "V");
    strcpy(circuit[4][3], "c");

    while(1)
    {
        if(DEV_Digital_Read(right) == 0){
            cursor_x += 1;
            if(cursor_x > 13){
                cursor_x = 0;
            }
        }
        if(DEV_Digital_Read(left) == 0){
            cursor_x -= 1;
            if(cursor_x < 0){
                cursor_x = 13;
            }
        }
        if(DEV_Digital_Read(dowm) == 0){
            cursor_y += 1;
            if(cursor_y > 3){
                cursor_y = 3;
            }
        }
        if(DEV_Digital_Read(up) == 0){
            cursor_y -= 1;
            if(cursor_y < 0){
                cursor_y = 0;
            }
        }
        if(DEV_Digital_Read(ctrl) == 0){
            next_gate(circuit[cursor_x][cursor_y]);
        }
        if(DEV_Digital_Read(keyA) == 0){
            // do measurement! 
            measure(circuit, stateVec);
            draw_results(stateVec, BlackImage);
            //reset the stateVec
            //for (int i = 0; i < 16; i++)
            //{
            //    stateVec[i] = 0;
            //}
            //stateVec[0] = 1;
        }
        if(DEV_Digital_Read(keyB) == 0){
            init_circuit(circuit);
        }
        if(DEV_Digital_Read(keyX) == 0){
            draw_help_gates(BlackImage);
        }
        // Display the current state
        // populating a screen in a 4 x 4 grid
        // 14 pxwide, 16px high. 
        draw_circuit(circuit, BlackImage);

        // TODO cursor stuff
        Paint_DrawString_EN(cursor_x*16, cursor_y*20, circuit[cursor_x][cursor_y], &Font20, BLACK, WHITE);
        //LCD_1IN14_Display(BlackImage);
        LCD_1IN3_Display(BlackImage);
        DEV_Delay_ms(50);
    }

    /* Module Exit */
    free(BlackImage);
    BlackImage = NULL;
    DEV_Module_Exit();
    return 0;
}
