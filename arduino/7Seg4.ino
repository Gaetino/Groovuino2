boolean digit[4][7];
int pin_digit[7] = {11,7,4,2,1,10,5};
int pin_select[4] = {6,8,9,12};


void load_digit(int digit_pos, int digit_num)
{
  if(digit_num==0) digit[digit_pos] = {1,1,1,1,1,1,0};
  if(digit_num==1) digit[digit_pos] = {0,1,1,0,0,0,0};
	if(digit_num==2) digit[digit_pos] = {1,1,0,1,1,0,1};
	if(digit_num==3) digit[digit_pos] = {1,1,1,1,0,0,1};
	if(digit_num==4) digit[digit_pos] = {0,1,1,0,0,1,1};
	if(digit_num==5) digit[digit_pos] = {1,0,1,1,0,1,1};
	if(digit_num==6) digit[digit_pos] = {1,0,1,1,1,1,1};
	if(digit_num==7) digit[digit_pos] = {1,1,1,0,0,0,0};
	if(digit_num==8) digit[digit_pos] = {1,1,1,1,1,1,1};
	if(digit_num==9) digit[digit_pos] = {1,1,1,1,0,1,1};
}

void print_all()
{
  for(int i=0; i<4; i++)
	{
	  digitalWrite(pin_select[0], HIGH);
		digitalWrite(pin_select[1], HIGH);
		digitalWrite(pin_select[2], HIGH);
		digitalWrite(pin_select[3], HIGH);
		for(int j=0; j<7; j++)
	  {
			digitalWrite(pin_digit[j], LOW);
		}
	  digitalWrite(pin_select[i], LOW);
	  for(int j=0; j<7; j++)
	  {
		  if(digit[i][j]) digitalWrite(pin_digit[j], HIGH);
		}
		delay(5);
	}
}

void print_value(int val)
{
	int mille = val/1000;
	load_digit(0,mille);
	
	int cent = (val-mille)/100;
	load_digit(1,cent);
	
	int dix = (val-mille-cent)/10;
	load_digit(2,dix);
	
	int unit = val-mille-cent-dix;
	load_digit(3,unit);
}

void loop()
{
	print_all();
}
