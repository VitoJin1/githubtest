/**
  ******************************************************************************
  * @file     
  * @author  Tmax Sco
  * @version V1.0.0
  * @date    2017-2-22
  * @brief   
  ******************************************************************************
  * @attention
  *
  *
  *
  * 
  ******************************************************************************
  */ 
/* Includes -------------------------------------------------------------------*/
#include "ctrl.h"
#include "math.h"
#include "rm_motor.h"
#include "usart.h"
#include "elmo.h"
#include "comm.h"
#include "timer.h"
#include "TLE5012.h"
#include "four_leg.h"
/* Private  typedef -----------------------------------------------------------*/
/* Private  define ------------------------------------------------------------*/
/* Private  macro -------------------------------------------------------------*/
/* Private  variables ---------------------------------------------------------*/
DriverType Driver[8] = {0};
/* Extern   variables ---------------------------------------------------------*/
extern MotorType Motor[8];
/* Extern   function prototypes -----------------------------------------------*/
/* Private  function prototypes -----------------------------------------------*/
/* Private  functions ---------------------------------------------------------*/


/**
  * @brief  驱动器初始化
  * @param  None
  * @retval 
  */
void DriverInit(void)
{
	Motor[COAXE_MOTOR_NUM].type = RM_3508;
	Motor[KNEE_MOTOR_NUM].type = RM_3508;
	Motor[SHOULDER_MOTOR_NUM].type = RM_3508;
	Motor[3].type = NONE;

	
	
#if BOARD == LEFT_FORWARD_LEG
	Driver[SHOULDER_MOTOR_NUM].command.canId = 1;
	Driver[COAXE_MOTOR_NUM].command.canId = 2;
	Driver[KNEE_MOTOR_NUM].command.canId = 3;
#elif BOARD == RIGHT_FORWARD_LEG
	Driver[SHOULDER_MOTOR_NUM].command.canId = 4;
	Driver[COAXE_MOTOR_NUM].command.canId = 5;
	Driver[KNEE_MOTOR_NUM].command.canId = 6;
#elif BOARD == LEFT_BACKWARD_LEG
	Driver[SHOULDER_MOTOR_NUM].command.canId = 7;
	Driver[COAXE_MOTOR_NUM].command.canId = 8;
	Driver[KNEE_MOTOR_NUM].command.canId = 9;	
#elif BOARD == RIGHT_BACKWARD_LEG
	Driver[SHOULDER_MOTOR_NUM].command.canId = 10;
	Driver[COAXE_MOTOR_NUM].command.canId = 11;
	Driver[KNEE_MOTOR_NUM].command.canId = 12;
#endif
	
	for(int i = 0; i < 8; i++)
	{
		Driver[i].status = DISABLE;
		Driver[i].encoder.period = 8192;		
		
	  if(Motor[i].type == RM_3508)
		{
			//Driver[i].unitMode = HOMING_MODE;
		 // Driver[i].unitMode = POSITION_CONTROL_MODE;
		 // Driver[i].unitMode = SPEED_CONTROL_MODE;
			Driver[i].unitMode = PT_MODE;
			Driver[i].velCtrl.kp = VEL_KP_3508 * 1;
			Driver[i].velCtrl.ki = VEL_KI_3508 * 1;
			Driver[i].velCtrl.maxOutput = CURRENT_MAX_3508;
			Driver[i].velCtrl.desiredVel[MAX_V] = VEL_MAX_3508;
			Driver[i].posCtrl.kd = POS_KD_3508;
			Driver[i].posCtrl.kp = POS_KP_3508;
			Driver[i].homingMode.current = 0.8f;
			
			Driver[i].velCtrl.acc = 3000.0f;
			Driver[i].velCtrl.dec = 3000.0f;
			Driver[i].velCtrl.desiredVel[CMD] = 0.0f;
			Driver[i].posCtrl.desiredPos = 0.0f;
			Driver[i].posCtrl.acc = Driver[i].velCtrl.dec;
			Driver[i].posCtrl.posVel = 250.0f;
			Driver[i].homingMode.vel = -160.0f;

		}
		else if(Motor[i].type == M_2006)  //M2006的参数
		{
			Driver[i].unitMode = HOMING_MODE;
		//  Driver[i].unitMode = POSITION_CONTROL_MODE;
		//  Driver[i].unitMode = SPEED_CONTROL_MODE;
			
			Driver[i].velCtrl.kp = VEL_KP_2006;
			Driver[i].velCtrl.ki = VEL_KI_2006;
			Driver[i].velCtrl.maxOutput = CURRENT_MAX_2006;		
			Driver[i].velCtrl.desiredVel[MAX_V] = VEL_MAX_2006;
			Driver[i].posCtrl.kd = POS_KD_2006;
			Driver[i].posCtrl.kp = POS_KP_2006;
			Driver[i].homingMode.current = 2.8f;
			
			Driver[i].velCtrl.acc = 100.0f;
			Driver[i].velCtrl.dec = 100.0f;
			Driver[i].velCtrl.desiredVel[CMD] = 250.0f;
			Driver[i].posCtrl.desiredPos = 0.0f;
			Driver[i].posCtrl.acc = 0.7f*Driver[i].velCtrl.dec;
			Driver[i].posCtrl.posVel = 250.0f;
			Driver[i].homingMode.vel = -160.0f;
		}
		else
		{
			break;
		}
	}
	Driver[SHOULDER_MOTOR_NUM].unitMode = POSITION_CONTROL_MODE;	
		
#if BOARD == AUTO_3508
	//自动车俯仰正转归位
	Driver[2].unitMode = POSITION_CONTROL_MODE;
	Driver[1].ptCtrl.velLimit = VEL_MAX_3508;
	Driver[0].ptCtrl.velLimit = VEL_MAX_3508;
	Driver[0].ptCtrl.motorNum = 2;
//	Driver[0].unitMode = HOMING_MODE;
#elif BOARD == AUTO_2006
	Driver[0].unitMode = HOMING_MODE;
;
#endif
	

}

/**
  * @brief  ZeroPosInit
	* @param  using the outer encoder to get the zero point of the motor
  * @note   There will be 0.4 degree of errors while zeropos initing
	* @retval None
  */
void ZeroPosInit(void)
{
#if BOARD == LEFT_FORWARD_LEG	
		Driver[SHOULDER_MOTOR_NUM].target5012B = 6981; 
#elif BOARD ==RIGHT_FORWARD_LEG
		Driver[SHOULDER_MOTOR_NUM].target5012B = 8041;
#elif BOARD == LEFT_BACKWARD_LEG
		Driver[SHOULDER_MOTOR_NUM].target5012B = 2648;
#elif BOARD == RIGHT_BACKWARD_LEG
		Driver[SHOULDER_MOTOR_NUM].target5012B = 506;
#endif
		Driver[SHOULDER_MOTOR_NUM].encoder5012B = TLE5012B_GetPos14bit();
		Driver[SHOULDER_MOTOR_NUM].posCtrl.actualPos = -M3508_RATIO *  (( Driver[SHOULDER_MOTOR_NUM].encoder5012B) - Driver[SHOULDER_MOTOR_NUM].target5012B); 
		Driver[SHOULDER_MOTOR_NUM].posCtrl.desiredPos = 0;
}

void ZeroPosCtrl(DriverType* driver)
{
	TLE5012B_UpateData();
	Driver[0].encoder5012B = TLE5012B_GetPos15bit()/4;
	PosCtrl(&driver->posCtrl);
}

/**
  * @brief  MotorCtrl
	* @param  None
	* @retval None
  */
float PerCur[4] = {0.0f};

void MotorCtrl(void)
{
//	CalculSpeed();
	
	TLE5012B_UpateData();
	Driver[SHOULDER_MOTOR_NUM].encoder5012B = TLE5012B_GetPos14bit();
	for(int i = 0; i < 8; i++)
	{
		if(Motor[i].type == NONE)
			break;
		
		CalculSpeed_Pos(&Driver[i],&Motor[i]);
		
		if(Driver[i].status != ENABLE)   
		{
			Driver[i].output = 0.0f;		
			continue;
		}
		if(CheckPtFlag(i,RECEIVE_BEGIN))
		{
			PtSecondBufferHandler(i);
		}
		//USART_OUT(USART3,(uint8_t*)"%d\r\n",Driver[KNEE_MOTOR_NUM].ptCtrl.executeFlag);	
		switch(Driver[i].unitMode)
		{
			case POSITION_CONTROL_MODE:
				//新版本位置环计算，使用斜坡
				PosCtrl(&Driver[i].posCtrl);
				Driver[i].velCtrl.desiredVel[CMD] = Driver[i].posCtrl.output;
				VelSlope(&Driver[i].velCtrl);
				Driver[i].output = VelPidCtrl(&Driver[i].velCtrl);
				break;
			case SPEED_CONTROL_MODE:
//				Driver[i].output = VelCtrl(VelSlope(Driver[i].velCtrl.desiredVel[CMD]));
				USART_OUT(USART3,(uint8_t*)"%d\t%d\r\n",(int)Driver[0].velCtrl.speed,(int)Driver[0].velCtrl.desiredVel[SOFT]);
				VelSlope(&Driver[i].velCtrl);
				Driver[i].output = VelPidCtrl(&Driver[i].velCtrl);
				break;
			case HOMING_MODE:
				HomingMode(&Driver[i]);
				Driver[i].output = Driver[i].homingMode.output;
				break;
			case PT_MODE:
				if(CheckPtFlag(i,RECEIVE_BEGIN))
				{
					PtSecondBufferHandler(i);
				}
			  PTCtrl(i,&Driver[i].ptCtrl,&Driver[i].posCtrl,&Driver[i].velCtrl);
				Driver[i].velCtrl.desiredVel[CMD] = Driver[i].ptCtrl.output;
				//PtVelSlope(i,&Driver[i].velCtrl,&Driver[i].ptCtrl);
				VelSlope(&Driver[i].velCtrl);
				Driver[i].output = VelPidCtrl(&Driver[i].velCtrl);		
				break;
			default:break;
		}

	}


  
//	PerCur[0] = Driver[0].output;
//	PerCur[1] = Driver[1].output;
//	PerCur[2] = Driver[2].output;
//	PerCur[3] = Driver[3].output;
//	PerCur[0] = 0.0f;
	
	for(int i = 0; i < 4; i++)
	{
		if(Motor[i].type == RM_3508)
			PerCur[i] = Driver[i].output*16384.0f/20.0f;
		else if(Motor[i].type == M_2006)
			PerCur[i] = Driver[i].output*10000.0f/10.0f;  //M2006
		else 
			PerCur[i] = 0.0f;
	}
	SetCur(PerCur);
//	USART_OUT(USART3,"%d\t",(int)PerCur[0]);
//	USART_OUT(USART3,(uint8_t*)"%d\t%d\r\n",(int)Driver[0].velCtrl.speed,(int)PerCur[0]);	
//	DMA_Send_Data((int)(Driver[0].velCtrl.speed) ,(int)(Driver[0].output*100.0f));
//	DMA_Send_Data((int)(Driver[2].velCtrl.speed) ,(int)(Driver[2].posCtrl.actualPos/10.0f));
//	DMA_Send_Data((int)(Driver[2].velCtrl.speed) ,(int)(Driver[2].posCtrl.output));
//	DMA_Send_Data((int)(Driver[0].velCtrl.speed) ,(int)(Driver[0].output*10.0f));
	
}

/**
  * @brief  虣讏斜莻摔色
  * @param  None
  * @retval 虣讏菤位摔远
  */
float VelSlope(VelCtrlType *velPid)
{
	/*************輪蹋輷莸虣讏斜莻**************/
	if(velPid->desiredVel[SOFT] < (velPid->desiredVel[CMD] - velPid->acc)){
		velPid->desiredVel[SOFT] +=velPid->acc;
	}else if(velPid->desiredVel[SOFT] > (velPid->desiredVel[CMD] + velPid->dec)){
		velPid->desiredVel[SOFT] -=velPid->dec;
	}else{
		velPid->desiredVel[SOFT] = velPid->desiredVel[CMD];
	}	
	return velPid->desiredVel[SOFT];
}


/**
  * @brief  速度控制
  * @param  None
  * @retval 速度PID的输出
  */
float VelPidCtrl(VelCtrlType *velPid)
{
	/*****************速度环PID*****************/
	velPid->velErr = velPid->desiredVel[SOFT] - (velPid->speed) ;	
	//计算积分
	velPid->iOut += velPid->ki * velPid->velErr;
	//积分限幅
	velPid->iOut = MaxMinLimit(velPid->iOut,velPid->maxOutput);
	//计算输出
	velPid->output = velPid->kp * velPid->velErr + velPid->iOut;
	//输出限幅
	velPid->output = MaxMinLimit(velPid->output,velPid->maxOutput);
	
	return velPid->output;
}

/**
  * @brief  速度环初始化
  * @param  None
  * @retval None 
  */
void VelCtrlInit(void)
{

}
/**
  * @brief  限制输出幅值
  * @param  val：输入值
  * @retval 输出值
  */
float OutPutLim(float value)
{
	float outputMax,outputMin,outputBasic;
	/********************计算动态最大最小输出****************************/
	outputBasic = Driver[0].velCtrl.speed * EMF_CONSTANT;								//估算反电动势
	outputMax = outputBasic + VOL_AMP;									//输出幅度
	outputMin = outputBasic - VOL_AMP;	 								//需要根据速度与电压关系改变
	if(outputMax <  VOL_AMP) outputMax =  VOL_AMP;			//
	if(outputMin > -VOL_AMP) outputMin = -VOL_AMP;
		
	if(value < outputMin)	value = outputMin;						//
	if(value > outputMax)	value = outputMax;

	if(value > VOL_MAX) value = VOL_MAX;
	if(value <-VOL_MAX) value =-VOL_MAX;

//	CurrentOutput = (value - (float)velpms*0.04315f)*25.0f;
	if(value < 0)	  value -= VOL_BLIND_AREA;												//消除控制盲区0.3043f Vq发布给tim4
	else       	 		value += VOL_BLIND_AREA;

	return value;
}

/**
  * @brief  位置控制(新位置环程序)
  * @param  None
  * @retval 位置环PID的输出。
  */
float PosCtrl(PosCtrlType *posPid)
{
	float posPidOut = 0.0f;
	float desiredVel = 0.0f,signVel = 1.0f;
	
	/******************************计算位置环输出**************************************/
	posPid->posErr = posPid->desiredPos - posPid->actualPos;				
	posPidOut = posPid->posErr*posPid->kp + posPid->kd*(posPid->posErr-posPid->posErrLast);		
	posPid->posErrLast = posPid->posErr;
	
	if(posPid->posErr < 0.0f) signVel = -1.0f;	
	
	//乘以0.7是因为减速需要有裕量，有待优化（斜坡问题）
	desiredVel = signVel*__sqrtf(2.0f*0.7f*posPid->acc*signVel*posPid->posErr);
		
	if(fabsf(desiredVel) < fabsf(posPidOut))
		posPidOut = desiredVel;
	//给一定大小的死区
//	if(fabsf(posPid->posErr) <= 200.0f)		posPidOut = 0.0f;
	
	posPid->output = MaxMinLimit(posPidOut,posPid->posVel);
	
	return posPid->output;
}

/**
  * @brief  BANG_BANG CONTROL FOR MAX VEL_LOOP OUTPUT
  * @param  None
  * @retval 虣讏菤位摔远
  */
#define DEAD_PERIOD 10
float PtVelSlope(uint8_t motorNum,VelCtrlType *velPid, PTCtrlType *ptPid)
{
	static uint8_t signVel,status;
	/*************輪蹋輷莸虣讏斜莻**************/
	if(CheckPtFlag(motorNum,BEGIN_MOTION))
	{
		//浠呭湪绱㈠紩鍙戠敓璺冲彉銆佹垨鑰呮湁鏂扮殑鏁版嵁杩涘叆鏃�
		if(CheckPtFlag(motorNum,INDEX_JUMP))
		{
			status = 0;
			SetPtFlag(motorNum,~INDEX_JUMP);
//			USART_OUT(USART3,"JUMP\t");
		}
		
		if( ptPid->index > 0)//涓嶄細鏁扮粍瓒婄晫锛屽湪PtCtrl涓紝瓒婁綅鍚庝細琚玦ndex浼氳缃浂
		{
			if(ptPid->desiredPos[POS_EXECUTOR][ptPid->index] - ptPid->desiredPos[POS_EXECUTOR][ptPid->index - 1] > 0)
			{
				signVel = 1;
			}
			else if(ptPid->desiredPos[ptPid->index] - ptPid->desiredPos[ptPid->index - 1] < 0)
			{
				signVel = 2;
			}
			
		}
		else if(ptPid->index == 0)//浠呴渶瑕佽�冭檻寰幆妯″紡锛堝洜涓鸿妯″紡涓婤EGIN_MOTION浼氳缃綅锛�
		{
			if(ptPid->velOutput > 0)
			{
				signVel = 1;
			}
			else if(ptPid->velOutput < 0)
			{
				signVel = 2;
			}
		}
		

		
			SetPtFlag(motorNum,~INDEX_JUMP);
			if(signVel == 1)
			{
				switch(status)
				{
					case 0:
						velPid->desiredVel[SOFT] = VEL_MAX_3508 / 1.f ;
						if(velPid->speed >= (ptPid->velOutput - DEAD_PERIOD) && ptPid -> cnt > 0)
						{
							velPid->desiredVel[SOFT] = velPid->desiredVel[CMD]; 
							status = 1;
						}
						break;
					case 1:
						velPid->desiredVel[SOFT] = velPid->desiredVel[CMD]; 
						break;
				}
			}
			else if(signVel == 2)
			{
				switch(status)
				{
					case 0:
						velPid->desiredVel[SOFT] = -VEL_MAX_3508 / 1.f ;
						if(velPid->speed <=(ptPid->velOutput + DEAD_PERIOD) && ptPid -> cnt > 0)
						{
							velPid->desiredVel[SOFT] = velPid->desiredVel[CMD];
							status = 1;
						}
						break;
					case 1:
						velPid->desiredVel[SOFT] = velPid->desiredVel[CMD];
						break;
				}
			}
		}
		else
		{
			velPid->desiredVel[SOFT] = velPid->desiredVel[CMD];
			SetPtFlag(motorNum,~INDEX_JUMP);
		}
//	USART_OUT(USART3,(uint8_t*)"%d\t%d\t%d\t%d\t%d\r\n",(int)ptPid->desiredPos[POS_EXECUTOR][ptPid->index],(int)posPid->actualPos,(int)(ptPid->velOutput),(int)velPid->speed,(int)velPid->desiredVel[SOFT]);	
//USART_OUT(USART3,(uint8_t*)"%d\t%d\t%d\t%d\r\n",(int)Driver[0].posCtrl.actualPos,(int)(Driver[0].ptCtrl.velOutput),(int)Driver[0].velCtrl.speed,(int)Driver[0].velCtrl.desiredVel[SOFT]);				
	return velPid->desiredVel[SOFT];
}

/**
  * @brief safety check
  * @param  
  * @retval 閫熷害杈撳嚭
  */
void PTSafetyCheck(uint8_t motorNum, PTCtrlType *ptPid)
{
	static int safety[8] = {0};
	if(ptPid->posMec > ptPid->pulseMaxLimit)
	{
		safety[motorNum]++;
	}
	else if(ptPid->posMec < ptPid->pulseMinLimit)
	{
		safety[motorNum]++;
	}
	if(safety[motorNum] > 30)	
	{
		ptPid->output = 0;
		ptPid->velOutput = 0;
		ptPid->posOutput = 0;
		Driver[motorNum].status = DISABLE;
		Driver[motorNum].output = 0;
	}		
}
	
	


/**
  * @brief PT鎻掑�艰繍琛岋紙鏈�澶т簩鍗佷釜鐐癸級
  * @param  
  * @retval 閫熷害杈撳嚭
  */
#define SATURATE(val,max,min)  ((val > max ? max : (val < min ? min : val)))
float PTCtrl(uint8_t motorNum, PTCtrlType *ptPid, PosCtrlType *posPid, VelCtrlType *velPid)
{
	static float kp[3] = {0.05,0.05,0.01},kd[3] = {0.03,0.03,0.01},ki= 0.000001,posErr[3] = {0},posErrLast[3] = {0} ,iout = 0;

	if(CheckPtFlag(motorNum,BEGIN_MOTION))
	{		
//		ptPid->desiredPos[POS_EXECUTOR][ptPid->index] = SATURATE(ptPid->desiredPos[POS_EXECUTOR][ptPid->index],ptPid->pulseMaxLimit,ptPid->pulseMinLimit);
		if(ptPid->index < ptPid->size)
		{
			if(ptPid->cnt < ptPid->desiredTime)
			{
				if(ptPid->index == 0)
				{
					ptPid->velOutput = ((ptPid->desiredPos[POS_EXECUTOR][ptPid->index] - posPid->actualPos)  / ptPid->desiredTime);
				}
				else
				{
					ptPid->velOutput = (ptPid->desiredPos[POS_EXECUTOR][ptPid->index] - ptPid->desiredPos[POS_EXECUTOR][ptPid->index-1])/ (ptPid->desiredTime);
				}				
				iout = MaxMinLimit(iout,VEL_MAX_3508 / 3.f);
				ptPid->cnt++;
				posErr[motorNum] = ptPid->desiredPos[POS_EXECUTOR][ptPid->index] - posPid->actualPos;
				iout += ki * posErr[motorNum];
				ptPid->posOutput = posErr[motorNum] * kp[motorNum] + (posErr[motorNum] - posErrLast[motorNum]) * kd[motorNum] + iout;  
				posErrLast[motorNum] = posErr[motorNum];				
			}
			else
			{
				ptPid->index++;
				SetPtFlag(motorNum,INDEX_JUMP);
				ptPid->cnt = 0;
				if(ptPid->index == ptPid->size)
				{
					SetPtFlag(motorNum,~BEGIN_MOTION);
					SetPtFlag(motorNum,ACTION_COMPLETE); //瀹屾垚鏍囧織浣嶈缃笂
				}
				if(ptPid->index >= ptPid->size - 3)//鍗冲皢瀹屾垚鏍囧織浣嶈缃笂
				{
					SetPtFlag(motorNum,ACTION_READY_TO_COMPLETE);
				}
			}
		}	
	}
	PtFirstBufferHandler(motorNum);	
	if(motorNum == KNEE_MOTOR_NUM)
		USART_OUT(USART3,(uint8_t*)"%d\t%d\t%d\r\n",(int)posPid->actualPos,(int)ptPid->desiredPos[POS_EXECUTOR][ptPid->index],(int)Driver[KNEE_MOTOR_NUM].output);
//		USART_OUT(USART3,(uint8_t*)"%d\t%d\r\n",(int)pPid->actualPos,(int)ptPid->cnt);
//		USART_OUT(USART3,(uint8_t*)"%d\t%d\t%d\t%d\t%d\r\n",(int)posPid->actualPos,(int)ptPid->desiredPos[POS_EXECUTOR][ptPid->index],(int)(ptPid->velOutput),(int)velPid->speed,(int)ptPid->velLimit);
	

	PTSafetyCheck(motorNum,ptPid);

	
	ptPid->output = MaxMinLimit(ptPid->velOutput + ptPid->posOutput,ptPid -> velLimit);

	return ptPid->output;
}

/**
  * @brief PT FlagManagement
  * @param  None
  * @retval 
  */
void SetPtFlag(uint8_t motorNum,uint32_t flag)
{
	switch(flag)
	{
		case SECOND_BUFFER_LOADING_CAN_BUFFER:
			Driver[motorNum].ptCtrl.executeFlag |= SECOND_BUFFER_LOADING_CAN_BUFFER;
			break;
		case ~SECOND_BUFFER_LOADING_CAN_BUFFER:
			Driver[motorNum].ptCtrl.executeFlag &= ~SECOND_BUFFER_LOADING_CAN_BUFFER;
			break;
		case FIRST_BUFFER_LOADING_SECOND_BUFFER:
			Driver[motorNum].ptCtrl.executeFlag |= FIRST_BUFFER_LOADING_SECOND_BUFFER;
			break;
		case ~FIRST_BUFFER_LOADING_SECOND_BUFFER:
			Driver[motorNum].ptCtrl.executeFlag &= ~FIRST_BUFFER_LOADING_SECOND_BUFFER;
			break;	
		case EXECUTOR_LOADING_FIRST_BUFFER:
			Driver[motorNum].ptCtrl.executeFlag |= EXECUTOR_LOADING_FIRST_BUFFER;
			break;
		case ~EXECUTOR_LOADING_FIRST_BUFFER:
			Driver[motorNum].ptCtrl.executeFlag &= ~EXECUTOR_LOADING_FIRST_BUFFER;
			break;
		case RECEIVE_START_AND_MP:
			Driver[motorNum].ptCtrl.executeFlag |= RECEIVE_START_AND_MP;
			break;
		case ~RECEIVE_START_AND_MP:
			Driver[motorNum].ptCtrl.executeFlag &= ~RECEIVE_START_AND_MP;
			break;		
		case RECEIVE_QN:
			Driver[motorNum].ptCtrl.executeFlag |= RECEIVE_QN;
			break;
		case ~RECEIVE_QN:
			Driver[motorNum].ptCtrl.executeFlag &= ~RECEIVE_QN;
			break;	
		case RECEIVE_BEGIN:
			Driver[motorNum].ptCtrl.executeFlag |= RECEIVE_BEGIN;
			break;
		case ~RECEIVE_BEGIN:
			Driver[motorNum].ptCtrl.executeFlag &= ~RECEIVE_BEGIN;
			break;
		case NEW_DATA:
			Driver[motorNum].ptCtrl.executeFlag |= NEW_DATA;
			break;
		case ~NEW_DATA:
			Driver[motorNum].ptCtrl.executeFlag &= ~NEW_DATA;
			break;
		case BEGIN_MOTION:
			Driver[motorNum].ptCtrl.executeFlag |= BEGIN_MOTION;
			break;
		case ~BEGIN_MOTION:
			Driver[motorNum].ptCtrl.executeFlag &= ~BEGIN_MOTION;
			break;
		case ACTION_COMPLETE:
			Driver[motorNum].ptCtrl.executeFlag |= ACTION_COMPLETE;
			break;
		case ~ACTION_COMPLETE:
			Driver[motorNum].ptCtrl.executeFlag &= ~ACTION_COMPLETE;
			break;		
		case ACTION_READY_TO_COMPLETE:
			Driver[motorNum].ptCtrl.executeFlag |= ACTION_READY_TO_COMPLETE;
			break;
		case ~ACTION_READY_TO_COMPLETE:
			Driver[motorNum].ptCtrl.executeFlag &= ~ACTION_READY_TO_COMPLETE;
			break;
		case INDEX_JUMP:
			Driver[motorNum].ptCtrl.executeFlag |= INDEX_JUMP;
			break;
		case ~INDEX_JUMP:
			Driver[motorNum].ptCtrl.executeFlag &= ~INDEX_JUMP;
			break;			
		case CAN_RECEIVING:
			Driver[motorNum].ptCtrl.executeFlag |= CAN_RECEIVING;
			break;
		case ~CAN_RECEIVING:
			Driver[motorNum].ptCtrl.executeFlag &= ~CAN_RECEIVING;
			break;
	}
}
uint8_t CheckPtFlag(uint8_t motorNum ,uint32_t flag)
	{
		if(Driver[motorNum].ptCtrl.executeFlag & flag)
		{
			return 1;
		}
		return 0;
}


/**
  * @brief  Homing mode
  * @param  None
  * @retval 输出的值
  */

void HomingMode(DriverType *driver)
{
	float output;

	driver->velCtrl.desiredVel[SOFT] = driver->homingMode.vel;
	output = VelPidCtrl(&driver->velCtrl);
	
	driver->homingMode.output = MaxMinLimit(output,driver->homingMode.current);//限制home模式时电流值
	
	if(fabsf(driver->velCtrl.speed) <=2){		//2
		driver->homingMode.cnt++;
	}else{
		driver->homingMode.cnt = 0;
	}
	
	if(driver->homingMode.cnt >= 500){									//500ms

		driver->posCtrl.actualPos=0.0f;				//
		driver->posCtrl.desiredPos = driver->posCtrl.actualPos + 8192.0f;
		//清除输出
		driver->homingMode.output = 0.0f;
		driver->velCtrl.desiredVel[CMD] = 0.0f;
		driver->velCtrl.desiredVel[SOFT] = 0.0f;
		driver->velCtrl.output = 0.0f;
		driver->output = 0.0f;
		driver->homingMode.output = 0.0f;
		driver->velCtrl.iOut = 0.0f;
		driver->unitMode = POSITION_CONTROL_MODE;
	}
}
/**
  * @brief  Homing mode Init
  * @param  None
  * @retval 输出的值
  */
void HomingModeInit(void)
{
}

/**
  * @brief  传递输出电压
  * @param  None
  * @retval 位置环输出的值
  */
float GetPosPidOut(void)
{
	return Driver[0].posCtrl.output;
}

/**
  * @brief  位置环初始化
  * @param  None
  * @retval None
  */
void PosCtrlInit(void)
{					

}

/**
  * @brief  Calculate Speed
  * @param  None
  * @retval Subtraction number between every two times.
**/
float CalculSpeed_Pos(DriverType *driver,MotorType *motor)
{
	int deltaPos = 0;
	deltaPos = (motor->pos - motor->posLast);
	motor->posLast = motor->pos;
	if(deltaPos > (driver->encoder.period/2)) deltaPos -= driver->encoder.period;
	if(deltaPos <-(driver->encoder.period/2)) deltaPos += driver->encoder.period;
	
	driver->posCtrl.actualPos += deltaPos;
	driver->ptCtrl.posMec += deltaPos;
	//用反馈速度输入
	driver->velCtrl.speed = (float)(motor->vel)*0.1365333f;					//1/60*8192/1000=0.136533
	//用位置差分出的速度输入
//	driver->velCtrl.speed = speed;
	
	return driver->velCtrl.speed; 
}
/**
  * @brief  Get Speed
  * @param  None
  * @retval Speed
**/
float GetSpeed(void)
{
	return Driver[0].velCtrl.speed;
}

/**
  * @brief  传递输出电压
  * @param  None
  * @retval 得到的值
  */
float GetVelPidOut(void)
{
	return Driver[0].velCtrl.output;
}

/**
  * @brief  max min limit
	* @param  inDat:
	* @retval outDat
  */
float MaxMinLimit(float val,float limit)
{
	if(val > limit) val =  limit;
	if(val <-limit) val = -limit;
	
	return val;
}



/**
  * @brief  电机使能
  * @param  n:哪个电机  (0-7)
	* @retval None
  */
void MotorOn(int n)
{
 if(Driver[n].unitMode == POSITION_CONTROL_MODE)
    Driver[n].posCtrl.desiredPos = Driver[n].posCtrl.actualPos;
  if(Driver[n].unitMode == SPEED_CONTROL_MODE)
    Driver[n].velCtrl.desiredVel[CMD] = 0.0f;
  
  Driver[n].velCtrl.iOut = 0.0f;

  Driver[n].status = ENABLE;
}

/**
  * @brief  电机失能
  * @param  n:哪个电机  (0-7)
	* @retval None
  */
void MotorOff(int n)
{
  Driver[n].status = DISABLE;
}

/**
  * @brief  速度环测试
	* @param  vel：测试用速度大小
	* @param  tim：速度切换时间
	* @retval None
  */
void VelCtrlTest(float vel,int tim)
{
	Driver[0].velCtrl.desiredVel[CMD] = vel;
	TIM_Delayms(TIM3,tim);
	Driver[0].velCtrl.desiredVel[CMD] = -vel;
	TIM_Delayms(TIM3,tim);

}


/************************ (C) COPYRIGHT 2016 ACTION *****END OF FILE****/
