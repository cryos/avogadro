/*
 *	wiiuse
 *
 *	Written By:
 *		Michael Laforest	< para >
 *		Email: < thepara (--AT--) g m a i l [--DOT--] com >
 *
 *	Copyright 2006-2007
 *
 *      Copyright (c) 2010 Mickael Gadroy
 *
 *	This file is part of wiiuse.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *	$Header$
 *
 */

/**
 *	@file
 *	@brief Handles IR data.
 */

#define bool int
#define true 1
#define false 0
#define _X_ 0
#define _Y_ 1
#define NBAXE 2
#define NBPAIR 6
  
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef WIN32
	#include <unistd.h>
#endif

#include "definitions.h"
#include "wiiuse_internal.h"
#include "ir.h"

static int get_ir_sens(struct wiimote_t* wm, char** block1, char** block2);
static void interpret_ir_data(struct wiimote_t* wm);
static void fix_rotated_ir_dots(struct ir_dot_t* dot, float ang);
static void get_ir_dot_avg(struct ir_dot_t* dot, int* x, int* y);
static void reorder_ir_dots(struct ir_dot_t* dot);
static float ir_distance(struct ir_dot_t* dot);
static int ir_correct_for_bounds(int* x, int* y, enum aspect_t aspect, int offset_x, int offset_y);
static void ir_convert_to_vres(int* x, int* y, enum aspect_t aspect, int vx, int vy);

static void calculateAverage( int *nbDistCalculated_in, int *distIsCalculated_in, double **dist_in, double *averageX_out, double *averageY_out ) ;
static void calculateStdDeviation( int *nbDistCalculated_in, int *distIsCalculated_in, double **dist_in, double *averageX_int, double *averageY_in, double *stdDeviationX_out, double *stdDeviationY_out ) ;
static void calculateStdDeviationPredited( double *averageX_in, double *averageY_in, double *stdDevPreditedX_out, double *stdDevPredictedY_out ) ;
static int searchAberrantValue1( struct wiimote_t* wm, int *nbDistCalculated_in, int *distIsCalculated_in, double** dist_in, int *whichAberrantValue_out, int *ignoreAberrantDist_out ) ;
static void searchAberrantValue2( struct wiimote_t* wm, int *nbDistCalculated_in, int *distIsCalculated_in, double** dist_in ) ;

static void interpret_ir_data2(struct wiimote_t* wm) ;
static void interpret_ir_data3(struct wiimote_t* wm) ;

/**
 *	@brief	Set if the wiimote should track IR targets.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 *	@param status	1 to enable, 0 to disable.
 */
void wiiuse_set_ir(struct wiimote_t* wm, int status) {
	byte buf;
	char* block1 = NULL;
	char* block2 = NULL;
	int ir_level;

	if (!wm)
		return;

	/*
	 *	Wait for the handshake to finish first.
	 *	When it handshake finishes and sees that
	 *	IR is enabled, it will call this function
	 *	again to actually enable IR.
	 */
	if (!WIIMOTE_IS_SET(wm, WIIMOTE_STATE_HANDSHAKE_COMPLETE)) {
		WIIUSE_DEBUG("Tried to enable IR, will wait until handshake finishes.");
		WIIMOTE_ENABLE_STATE(wm, WIIMOTE_STATE_IR);
		return;
	}

	/*
	 *	Check to make sure a sensitivity setting is selected.
	 */
	ir_level = get_ir_sens(wm, &block1, &block2);
	if (!ir_level) {
		WIIUSE_ERROR("No IR sensitivity setting selected.");
		return;
	}

	if (status) {
		/* if already enabled then stop */
		if (WIIMOTE_IS_SET(wm, WIIMOTE_STATE_IR))
			return;
		WIIMOTE_ENABLE_STATE(wm, WIIMOTE_STATE_IR);
	} else {
		/* if already disabled then stop */
		if (!WIIMOTE_IS_SET(wm, WIIMOTE_STATE_IR))
			return;
		WIIMOTE_DISABLE_STATE(wm, WIIMOTE_STATE_IR);
	}

	/* set camera 1 and 2 */
	buf = (status ? 0x04 : 0x00);
	wiiuse_send(wm, WM_CMD_IR, &buf, 1);
	wiiuse_send(wm, WM_CMD_IR_2, &buf, 1);

	if (!status) {
		WIIUSE_DEBUG("Disabled IR cameras for wiimote id %i.", wm->unid);
		wiiuse_set_report_type(wm);
		return;
	}

	/* enable IR, set sensitivity */
	buf = 0x08;
	wiiuse_write_data(wm, WM_REG_IR, &buf, 1);

	/* write sensitivity blocks */
	wiiuse_write_data(wm, WM_REG_IR_BLOCK1, (byte*)block1, 9);
	wiiuse_write_data(wm, WM_REG_IR_BLOCK2, (byte*)block2, 2);

	/* set the IR mode */
	if (WIIMOTE_IS_SET(wm, WIIMOTE_STATE_EXP))
		buf = WM_IR_TYPE_BASIC;
	else
		buf = WM_IR_TYPE_EXTENDED;
	wiiuse_write_data(wm, WM_REG_IR_MODENUM, &buf, 1);

	/* set the wiimote report type */
	wiiuse_set_report_type(wm);

	WIIUSE_DEBUG("Enabled IR camera for wiimote id %i (sensitivity level %i).", wm->unid, ir_level);
}


/**
 *	@brief	Get the IR sensitivity settings.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 *	@param block1	[out] Pointer to where block1 will be set.
 *	@param block2	[out] Pointer to where block2 will be set.
 *
 *	@return Returns the sensitivity level.
 */
static int get_ir_sens(struct wiimote_t* wm, char** block1, char** block2) {
	if (WIIMOTE_IS_SET(wm, WIIMOTE_STATE_IR_SENS_LVL1)) {
		*block1 = WM_IR_BLOCK1_LEVEL1;
		*block2 = WM_IR_BLOCK2_LEVEL1;
		return 1;
	} else if (WIIMOTE_IS_SET(wm, WIIMOTE_STATE_IR_SENS_LVL2)) {
		*block1 = WM_IR_BLOCK1_LEVEL2;
		*block2 = WM_IR_BLOCK2_LEVEL2;
		return 2;
	} else if (WIIMOTE_IS_SET(wm, WIIMOTE_STATE_IR_SENS_LVL3)) {
		*block1 = WM_IR_BLOCK1_LEVEL3;
		*block2 = WM_IR_BLOCK2_LEVEL3;
		return 3;
	} else if (WIIMOTE_IS_SET(wm, WIIMOTE_STATE_IR_SENS_LVL4)) {
		*block1 = WM_IR_BLOCK1_LEVEL4;
		*block2 = WM_IR_BLOCK2_LEVEL4;
		return 4;
	} else if (WIIMOTE_IS_SET(wm, WIIMOTE_STATE_IR_SENS_LVL5)) {
		*block1 = WM_IR_BLOCK1_LEVEL5;
		*block2 = WM_IR_BLOCK2_LEVEL5;
		return 5;
	}

	*block1 = NULL;
	*block2 = NULL;
	return 0;
}


/**
 *	@brief	Set the virtual screen resolution for IR tracking.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 *	@param status	1 to enable, 0 to disable.
 */
void wiiuse_set_ir_vres(struct wiimote_t* wm, unsigned int x, unsigned int y) {
	if (!wm)	return;

	wm->ir.vres[0] = (x-1);
	wm->ir.vres[1] = (y-1);
}


/**
 *	@brief	Set the XY position for the IR cursor.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 */
void wiiuse_set_ir_position(struct wiimote_t* wm, enum ir_position_t pos) {

  int i ;	

  if (!wm)	return;

  //-> Initialisation for interpret_ir2
  puts( "wiiuse_set_ir_position() -> init var for interpret_ir_data2" ) ;

  for( i=0 ; i<WM_MAX_DOTS ; i++ )
  {
    wm->ir.lastRealDot[i][0] = -1 ;
    wm->ir.lastRealDot[i][1] = -1 ;
    wm->ir.lastRealDotIsVisible[i] = false ;
    //wm->ir.lastRealRealDot[i][0] = -1 ;
    //wm->ir.lastRealRealDot[i][1] = -1 ;
    wm->ir.persistanceTimer[i] = 0 ;
  }

  for( i=0 ; i<6 ; i++ )
    wm->ir.lastDist[i] = -1 ;

  wm->ir.szLastRealDot=0 ;
  wm->ir.szLastRealRealDot=0 ;
  wm->ir.szLastRealRealDot2=0 ;
  wm->ir.lastCursorRealRealReal[0]=-1 ;
  wm->ir.lastCursorRealRealReal[1]=-1 ;
  wm->ir.deltax = 0 ;
  wm->ir.deltay = 0 ;
  wm->ir.deltaz = 0 ;
  wm->ir.isInPrecisionMode = 0 ;
  wm->ir.x = 0 ;
  wm->ir.y = 0 ;
  wm->ir.z = 0 ;
  wm->ir.ax = 0 ;
  wm->ir.ay = 0 ;
  wm->ir.distance = 0 ;
  wm->orient.yaw = 0 ;
  //<-


	wm->ir.pos = pos;

	switch (pos) {

		case WIIUSE_IR_ABOVE:
			wm->ir.offset[0] = 0;

			if (wm->ir.aspect == WIIUSE_ASPECT_16_9)
				wm->ir.offset[1] = WM_ASPECT_16_9_Y/2 - 70;
			else if (wm->ir.aspect == WIIUSE_ASPECT_4_3)
				wm->ir.offset[1] = WM_ASPECT_4_3_Y/2 - 100;

			return;

		case WIIUSE_IR_BELOW:
			wm->ir.offset[0] = 0;

			if (wm->ir.aspect == WIIUSE_ASPECT_16_9)
				wm->ir.offset[1] = -WM_ASPECT_16_9_Y/2 + 100;
			else if (wm->ir.aspect == WIIUSE_ASPECT_4_3)
				wm->ir.offset[1] = -WM_ASPECT_4_3_Y/2 + 70;

			return;

		default:
			return;
	};
}


/**
 *	@brief	Set the aspect ratio of the TV/monitor.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 *	@param aspect	Either WIIUSE_ASPECT_16_9 or WIIUSE_ASPECT_4_3
 */
void wiiuse_set_aspect_ratio(struct wiimote_t* wm, enum aspect_t aspect) {
	if (!wm)	return;

	wm->ir.aspect = aspect;

	if (aspect == WIIUSE_ASPECT_4_3) {
		wm->ir.vres[0] = WM_ASPECT_4_3_X;
		wm->ir.vres[1] = WM_ASPECT_4_3_Y;
	} else {
		wm->ir.vres[0] = WM_ASPECT_16_9_X;
		wm->ir.vres[1] = WM_ASPECT_16_9_Y;
	}

	/* reset the position offsets */
	wiiuse_set_ir_position(wm, wm->ir.pos);
}


/**
 *	@brief	Set the IR sensitivity.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 *	@param level	1-5, same as Wii system sensitivity setting.
 *
 *	If the level is < 1, then level will be set to 1.
 *	If the level is > 5, then level will be set to 5.
 */
void wiiuse_set_ir_sensitivity(struct wiimote_t* wm, int level) {
	char* block1 = NULL;
	char* block2 = NULL;

	if (!wm)	return;

	if (level > 5)		level = 5;
	if (level < 1)		level = 1;

	WIIMOTE_DISABLE_STATE(wm, (WIIMOTE_STATE_IR_SENS_LVL1 |
								WIIMOTE_STATE_IR_SENS_LVL2 |
								WIIMOTE_STATE_IR_SENS_LVL3 |
								WIIMOTE_STATE_IR_SENS_LVL4 |
								WIIMOTE_STATE_IR_SENS_LVL5));

	switch (level) {
		case 1:
			WIIMOTE_ENABLE_STATE(wm, WIIMOTE_STATE_IR_SENS_LVL1);
			break;
		case 2:
			WIIMOTE_ENABLE_STATE(wm, WIIMOTE_STATE_IR_SENS_LVL2);
			break;
		case 3:
			WIIMOTE_ENABLE_STATE(wm, WIIMOTE_STATE_IR_SENS_LVL3);
			break;
		case 4:
			WIIMOTE_ENABLE_STATE(wm, WIIMOTE_STATE_IR_SENS_LVL4);
			break;
		case 5:
			WIIMOTE_ENABLE_STATE(wm, WIIMOTE_STATE_IR_SENS_LVL5);
			break;
		default:
			return;
	}

	/* set the new sensitivity */
	get_ir_sens(wm, &block1, &block2);

	wiiuse_write_data(wm, WM_REG_IR_BLOCK1, (byte*)block1, 9);
	wiiuse_write_data(wm, WM_REG_IR_BLOCK2, (byte*)block2, 2);

	WIIUSE_DEBUG("Set IR sensitivity to level %i (unid %i)", level, wm->unid);
}


/**
 *	@brief Calculate the data from the IR spots.  Basic IR mode.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 *	@param data		Data returned by the wiimote for the IR spots.
 */
void calculate_basic_ir(struct wiimote_t* wm, byte* data) {
	struct ir_dot_t* dot = wm->ir.dot;
	int i;

	dot[0].rx = 1023 - (data[0] | ((data[2] & 0x30) << 4));
	dot[0].ry = data[1] | ((data[2] & 0xC0) << 2);

	dot[1].rx = 1023 - (data[3] | ((data[2] & 0x03) << 8));
	dot[1].ry = data[4] | ((data[2] & 0x0C) << 6);

	dot[2].rx = 1023 - (data[5] | ((data[7] & 0x30) << 4));
	dot[2].ry = data[6] | ((data[7] & 0xC0) << 2);

	dot[3].rx = 1023 - (data[8] | ((data[7] & 0x03) << 8));
	dot[3].ry = data[9] | ((data[7] & 0x0C) << 6);

	/* set each IR spot to visible if spot is in range */
	for (i = 0; i < 4; ++i) {
		if (dot[i].ry == 1023)
			dot[i].visible = 0;
		else {
			dot[i].visible = 1;
			dot[i].size = 0;		/* since we don't know the size, set it as 0 */
		}
	}

	//interpret_ir_data(wm) ;
	//interpret_ir_data2(wm) ;
  interpret_ir_data3(wm) ;
}


/**
 *	@brief Calculate the data from the IR spots.  Extended IR mode.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 *	@param data		Data returned by the wiimote for the IR spots.
 */
void calculate_extended_ir(struct wiimote_t* wm, byte* data) {
	struct ir_dot_t* dot = wm->ir.dot;
	int i;

	for (i = 0; i < 4; ++i) {
		dot[i].rx = 1023 - (data[3*i] | ((data[(3*i)+2] & 0x30) << 4));
		dot[i].ry = data[(3*i)+1] | ((data[(3*i)+2] & 0xC0) << 2);

		dot[i].size = data[(3*i)+2] & 0x0F;

		/* if in range set to visible */
		if (dot[i].ry == 1023)
			dot[i].visible = 0;
		else
			dot[i].visible = 1;
	}

	//interpret_ir_data(wm) ;
	//interpret_ir_data2(wm) ;
  interpret_ir_data3(wm) ;
}


/**
 *	@brief Interpret IR data into more user friendly variables.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 */
static void interpret_ir_data(struct wiimote_t* wm) {
	struct ir_dot_t* dot = wm->ir.dot;
	int i;
	float roll = 0.0f;
	int last_num_dots = wm->ir.num_dots;

	if (WIIMOTE_IS_SET(wm, WIIMOTE_STATE_ACC))
		roll = wm->orient.roll;

	// count visible dots
	wm->ir.num_dots = 0;
	for (i = 0; i < 4; ++i) {
		if (dot[i].visible)
			wm->ir.num_dots++;
	}

	switch (wm->ir.num_dots) {
		case 0:
		{
			wm->ir.state = 0;

			// reset the dot ordering
			for (i = 0; i < 4; ++i)
				dot[i].order = 0;

			wm->ir.x = 0;
			wm->ir.y = 0;
			wm->ir.z = 0.0f;

			return;
		}
		case 1:
		{
			fix_rotated_ir_dots(wm->ir.dot, roll);

			if (wm->ir.state < 2) {
				
				// Only 1 known dot, so use just that.
				 
				for (i = 0; i < 4; ++i) {
					if (dot[i].visible) {
						wm->ir.x = dot[i].x;
						wm->ir.y = dot[i].y;

						wm->ir.ax = wm->ir.x;
						wm->ir.ay = wm->ir.y;

						//	can't calculate yaw because we don't have the distance
						//wm->orient.yaw = calc_yaw(&wm->ir);

						ir_convert_to_vres(&wm->ir.x, &wm->ir.y, wm->ir.aspect, wm->ir.vres[0], wm->ir.vres[1]);
						break;
					}
				}
			} else {
				
				// 	Only see 1 dot but know theres 2.
				//	Try to estimate where the other one
				// 	should be and use that.
				 
				for (i = 0; i < 4; ++i) {
					if (dot[i].visible) {
						int ox = 0;
						int x, y;

						if (dot[i].order == 1)
							// visible is the left dot - estimate where the right is
							ox = dot[i].x + wm->ir.distance;
						else if (dot[i].order == 2)
							// visible is the right dot - estimate where the left is
							ox = dot[i].x - wm->ir.distance;

						x = ((signed int)dot[i].x + ox) / 2;
						y = dot[i].y;

						wm->ir.ax = x;
						wm->ir.ay = y;
						wm->orient.yaw = calc_yaw(&wm->ir);

						if (ir_correct_for_bounds(&x, &y, wm->ir.aspect, wm->ir.offset[0], wm->ir.offset[1])) {
							ir_convert_to_vres(&x, &y, wm->ir.aspect, wm->ir.vres[0], wm->ir.vres[1]);
							wm->ir.x = x;
							wm->ir.y = y;
						}

						break;
					}
				}
			}

			break;
		}
		case 2:
		case 3:
		case 4:
		{
			
			//	Two (or more) dots known and seen.
			//	Average them together to estimate the true location.
			
			int x, y;
			wm->ir.state = 2;

			fix_rotated_ir_dots(wm->ir.dot, roll);

			// if there is at least 1 new dot, reorder them all
			if (wm->ir.num_dots > last_num_dots) {
				reorder_ir_dots(dot);
				wm->ir.x = 0;
				wm->ir.y = 0;
			}

			wm->ir.distance = ir_distance(dot);
			wm->ir.z = 1023 - wm->ir.distance;

			get_ir_dot_avg(wm->ir.dot, &x, &y);

			wm->ir.ax = x;
			wm->ir.ay = y;
			wm->orient.yaw = calc_yaw(&wm->ir);

			if (ir_correct_for_bounds(&x, &y, wm->ir.aspect, wm->ir.offset[0], wm->ir.offset[1])) {
				ir_convert_to_vres(&x, &y, wm->ir.aspect, wm->ir.vres[0], wm->ir.vres[1]);
				wm->ir.x = x;
				wm->ir.y = y;
			}

			break;
		}
		default:
		{
			break;
		}
	}

	#ifdef WITH_WIIUSE_DEBUG
	{
	int ir_level;
	WIIUSE_GET_IR_SENSITIVITY(wm, &ir_level);
	WIIUSE_DEBUG("IR sensitivity: %i", ir_level);
	WIIUSE_DEBUG("IR visible dots: %i", wm->ir.num_dots);
	for (i = 0; i < 4; ++i)
		if (dot[i].visible)
			WIIUSE_DEBUG("IR[%i][order %i] (%.3i, %.3i) -> (%.3i, %.3i)", i, dot[i].order, dot[i].rx, dot[i].ry, dot[i].x, dot[i].y);
	WIIUSE_DEBUG("IR[absolute]: (%i, %i)", wm->ir.x, wm->ir.y);
	}
	#endif
}

/**
 *	@brief Another method to interpret IR data which hope to give data stabler.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 */

static void interpret_ir_data2(struct wiimote_t* wm)
{
  // dots = LEDs
  // points = groups of dots

  // 1. Identification points.
  int realDot[WM_MAX_DOTS][2]={{0,0},{0,0},{0,0},{0,0}} ;
  int divMoy[WM_MAX_DOTS]={0,0,0,0} ;
  int szRealDot=0 ; // Nb boxes actually used. ((fr)actually->réellement)
  //int cursorReal[2]={-1,-1} ;

  // 2. Persistence points.
  bool isUpdate[WM_MAX_DOTS]={false,false,false,false} ;

  // 3. Etap to get cursor position.
  int realRealDot[WM_MAX_DOTS][2]={{-1,-1},{-1,-1},{-1,-1},{-1,-1}} ;
  int szRealRealDot=0 ; // Nb boxes actually used at the end.
  int cursorRealReal[2]={-1,-1} ;

  // 4. Last completion. ((fr)finalization)
  //int cursorRealRealReal[2]={-1,-1} ;

  // For intermediate needs.
  double dist=0.0 ;
  double a,b ; //,c ;
  int x, y ; //, z ;
  int i=0, j=0 ;
  bool newDot=true, needAdd=false ;
  struct ir_dot_t* dot=wm->ir.dot ;


  /// 1. Identification points.

  wm->ir.num_dots = 0 ;
	for( i=0 ; i<WM_MAX_DOTS; i++ )
	{
		if( dot[i].visible )
		{
      wm->ir.num_dots++ ;
			x = dot[i].rx ;
      y = dot[i].ry ;

      //printf( "Visible dots x:%d y:%d \n", x, y ) ;

      // No extrem data ...
      if( (x>0 || y>0) && (x<1022 || y<1022) )
      { 
        j = 0 ;
        newDot = true ;

        // Detection of points.
        while( j<szRealDot && newDot==true )
        {
          if( dot[j].visible )
          {
            a = (double)(x-dot[j].rx) ;
            b = (double)(y-dot[j].ry) ;
            dist = sqrt(a*a+b*b) ;

            if( dist < ERROR_BETWEEN_DOTS )
            { // Coupling dots.

              newDot = false ;
              realDot[j][0] += x ;
              realDot[j][1] += y ;
              divMoy[j]++ ;
            }
          }
          j++ ;
        }

        if( newDot == true )
        { // New points detected.

          realDot[szRealDot][0] = x ;
          realDot[szRealDot][1] = y ;
          divMoy[szRealDot]++ ;
          szRealDot++ ;
        }
      }
		}
	}

  // Calculate the averages to get points.
  for( i=0 ; i<szRealDot ; i++ )
  {
    realDot[i][0] = (int)((double)realDot[i][0]/(double)divMoy[i]) ;
    realDot[i][1] = (int)((double)realDot[i][1]/(double)divMoy[i]) ;
    //printf( "  Visible points x:%d y:%d \n", realDot[i][0], realDot[i][1] ) ;
  }


  /// 2. Persistence points.

  if( wm->ir.szLastRealDot == 0 )
  { // Copy the existing points.
    
    for( i=0 ; i<szRealDot ; i++ )
    {
      wm->ir.lastRealDot[i][0] = realDot[i][0] ;
      wm->ir.lastRealDot[i][1] = realDot[i][1] ;
      wm->ir.szLastRealDot++ ;
      wm->ir.persistanceTimer[i] = PERSISTENCE_CYCLE+ERROR_PERS_CYCLE ; // Close to being deleted.
    }
  }
  else
  {
    // Find matches.
    for( i=0 ; i<szRealDot ; i++ )
    {
      needAdd = true ;
      j = 0 ;
      while( j<wm->ir.szLastRealDot && needAdd==true )
      {
        if(    (realDot[i][0]<(wm->ir.lastRealDot[j][0]+ERROR_BETWEEN_DOTS)) 
            && (realDot[i][0]>(wm->ir.lastRealDot[j][0]-ERROR_BETWEEN_DOTS))
            && (realDot[i][1]<(wm->ir.lastRealDot[j][1]+ERROR_BETWEEN_DOTS)) 
            && (realDot[i][1]>(wm->ir.lastRealDot[j][1]-ERROR_BETWEEN_DOTS)) )
        { // Correspondence found.

          isUpdate[j] = true ;
          needAdd = false ;
          
          if( wm->ir.persistanceTimer[j] < (PERSISTENCE_CYCLE+ERROR_PERS_CYCLE) )
            wm->ir.persistanceTimer[j]++ ;

          wm->ir.lastRealDot[j][0] = realDot[i][0] ;
          wm->ir.lastRealDot[j][1] = realDot[i][1] ;
        }

        j++ ;
      }

      // If no match, add new points.
      if( needAdd )
      {
        wm->ir.lastRealDot[wm->ir.szLastRealDot][0] = realDot[i][0] ;
        wm->ir.lastRealDot[wm->ir.szLastRealDot][1] = realDot[i][1] ;
        wm->ir.persistanceTimer[wm->ir.szLastRealDot]++ ;
        isUpdate[wm->ir.szLastRealDot] = true ;
        wm->ir.szLastRealDot++ ;
      }
    }

    // Removing unused values.
    for( i=0 ; i<wm->ir.szLastRealDot ; i++ )
    {
      if( isUpdate[i] == false )
      {
        wm->ir.persistanceTimer[i]-- ;

        if( wm->ir.persistanceTimer[i] < PERSISTENCE_CYCLE )
        {
          wm->ir.lastRealDot[i][0] = 0 ;
          wm->ir.lastRealDot[i][1] = 0 ;
          wm->ir.persistanceTimer[i] = 0 ;
        }
      } 
    } // The rest just after.

    // Group data.
    wm->ir.szLastRealDot = 0 ;
    for( i=0 ; i<WM_MAX_DOTS ; i++ )
    {
      if( wm->ir.persistanceTimer[i] > 0 )
      {
        //printf( "2 %d wm->ir.lastRealDot x:%d y:%d\n", 
        //          i, wm->ir.lastRealDot[i][0], wm->ir.lastRealDot[i][1] ) ;

        if( i > wm->ir.szLastRealDot )
        {
          wm->ir.lastRealDot[wm->ir.szLastRealDot][0] = wm->ir.lastRealDot[i][0] ;
          wm->ir.lastRealDot[wm->ir.szLastRealDot][1] = wm->ir.lastRealDot[i][1] ;
          wm->ir.persistanceTimer[wm->ir.szLastRealDot] = wm->ir.persistanceTimer[i] ;

          wm->ir.lastRealDot[i][0] = 0 ;
          wm->ir.lastRealDot[i][1] = 0 ;
          wm->ir.persistanceTimer[i] = 0 ;
        }

        wm->ir.szLastRealDot++ ;
      }
    }
  } // if( wm->ir.szLastRealDot == 0 )

  
  //for( i=0 ; i<wm->ir.szLastRealDot ; i++ )
  //  printf( "      Persistent points x:%d y:%d \n", 
  //            wm->ir.lastRealDot[i][0], wm->ir.lastRealDot[i][1] ) ;

  /// 3. Etap to get cursor position.

  // If persistent data, get value.
  szRealRealDot = 0 ;
  for( i=0 ; i<wm->ir.szLastRealDot ; i++ )
  {
    if( wm->ir.persistanceTimer[i] >= PERSISTENCE_CYCLE )
    {
      realRealDot[szRealRealDot][0] = wm->ir.lastRealDot[i][0] ;
      realRealDot[szRealRealDot][1] = wm->ir.lastRealDot[i][1] ;
      szRealRealDot++ ;
    }
  }

  // Method : Average all dot.
  // Problem of this method when there is a lost/added dot.
  if( szRealRealDot > 0 )
  {
    cursorRealReal[0] = 0 ;
    cursorRealReal[1] = 0 ; // L'init. à -1 ne semble pas suffisant ...

    for( i=0 ; i<szRealRealDot ; i++ )
    {
      //printf( "3 %d realRealDot x:%d y:%d\n", i, realRealDot[i][0], realRealDot[i][1] ) ;
      cursorRealReal[0] += realRealDot[i][0] ;
      cursorRealReal[1] += realRealDot[i][1] ;
    }

    cursorRealReal[0] /= szRealRealDot ;
    cursorRealReal[1] /= szRealRealDot ;

    wm->ir.x = cursorRealReal[0] ;
    wm->ir.y = cursorRealReal[1] ;
  }
  //else
    // nothing, the values stay at -1.


  /*
  
  In commentary because, if an adjustment is realized when a LED is added or lost,
  an information is lost : the brutal change of the cursor position.
  So the final application do not know when the cursor becomes "crazy".
  Or this code must try to block the cursor "crazy jump".
  
  /// (4.) Last completion. ((fr)finalization)
  // Transition etap.
  // If the diffence between last and new cursor position is excessive,
  // reduce movement in direction of new cursor.

  if( cursorRealReal[0]>0 && cursorRealReal[0]<1022 && cursorRealReal[1]>0 && cursorRealReal[1]<1022 )
  {
    // 1st initialization.
    if( wm->ir.lastCursorRealRealReal[0]==-1 && wm->ir.lastCursorRealRealReal[1]==-1 )
    {
      wm->ir.lastCursorRealRealReal[0] = cursorRealReal[0] ;
      wm->ir.lastCursorRealRealReal[1] = cursorRealReal[1] ;
    }

    //printf( "  lastCursorRealRealReal x:%d y:%d \n", wm->ir.lastCursorRealRealReal[0], wm->ir.lastCursorRealRealReal[1] ) ;

    if( szRealRealDot==wm->ir.szLastRealRealDot2 )
    {
      cursorRealRealReal[0] = cursorRealReal[0] ;
      cursorRealRealReal[1] = cursorRealReal[1] ;
    }
    else
    { // Progress towards this.
      
      a = cursorRealReal[0] - wm->ir.lastCursorRealRealReal[0] ;
      b = cursorRealReal[1] - wm->ir.lastCursorRealRealReal[1] ;
      c = ((a>=0?a:a*(-1.0f))>(b>=0?b:b*(-1.0f)) ? a : b ) ;
      c = ( c>-1 && c<1 ? 1.0f : c ) ;
      //printf( "  a:%lf b:%lf c:%lf \n", a, b, c ) ;
      cursorRealRealReal[0] = wm->ir.lastCursorRealRealReal[0] + (int)(a/c*10.0) ; // A constante for a max movement.
      cursorRealRealReal[1] = wm->ir.lastCursorRealRealReal[1] + (int)(b/c*10.0) ;
    }
    
    if(    (wm->ir.lastCursorRealRealReal[0]<(cursorRealRealReal[0]+ERROR_BETWEEN_LAST_NEW_CURSOR)) 
        && (wm->ir.lastCursorRealRealReal[0]>(cursorRealRealReal[0]-ERROR_BETWEEN_LAST_NEW_CURSOR))
        && (wm->ir.lastCursorRealRealReal[1]<(cursorRealRealReal[1]+ERROR_BETWEEN_LAST_NEW_CURSOR))
        && (wm->ir.lastCursorRealRealReal[1]>(cursorRealRealReal[1]-ERROR_BETWEEN_LAST_NEW_CURSOR)) )
    { // The transition is no longer necessary.

       wm->ir.szLastRealRealDot2 = szRealRealDot ;
    }

    wm->ir.lastCursorRealRealReal[0] = cursorRealRealReal[0] ;
    wm->ir.lastCursorRealRealReal[1] = cursorRealRealReal[1] ;

    wm->ir.x = cursorRealRealReal[0] ;
    wm->ir.y = cursorRealRealReal[1] ;
  }
  else
  {
    
    if( !(wm->ir.lastCursorRealRealReal[0]==-1 && wm->ir.lastCursorRealRealReal[1]==-1) )
    {
      cursorRealRealReal[0] = wm->ir.lastCursorRealRealReal[0] ;
      cursorRealRealReal[1] = wm->ir.lastCursorRealRealReal[1] ;

      wm->ir.x = wm->ir.lastCursorRealRealReal[0] ;
	    wm->ir.y = wm->ir.lastCursorRealRealReal[1] ;
    }
    else
    { // Not initialize yet.

      wm->ir.x = 0 ;
	    wm->ir.y = 0 ;
    }      
  }
  */
  
  
  //if( cursorRealReal[0]>1 && cursorRealReal[1]>1 )
  //  printf( "        New points x:%d y:%d \n", wm->ir.x, wm->ir.y ) ;
  

  // (5.) Update different variables.
  if( wm->ir.num_dots >= 2 )
    wm->ir.state = 2 ;
  else
    wm->ir.state = 0 ;

  // Update absolute value.
  wm->ir.ax = 0 ;
  wm->ir.ay = 0 ;
  
  // Nb source detected.
  wm->ir.nb_source_detect = szRealRealDot ;

  if( szRealDot > 0 )
  {
    for( i=0 ; i<szRealDot ; i++ )
    {
      wm->ir.ax += realDot[i][0] ;
	    wm->ir.ay += realDot[i][1] ;
    }

    wm->ir.ax /= szRealDot ;
    wm->ir.ay /= szRealDot ;
  }
  
  //if( cursorRealReal[0]>1 && cursorRealReal[1]>1 )
  //  printf( "          Absolute points x:%d y:%d \n", wm->ir.ax, wm->ir.ay ) ;
  

  // Update dots.
  for( i=0, j=0 ; i<WM_MAX_DOTS ; i++ )
  {
    if( dot[i].visible )
    {
      wm->ir.dot[i].x = wm->ir.dot[i].rx ;
      wm->ir.dot[i].y = wm->ir.dot[i].ry ;
    }
  }

  // Update distance & z.
  if( !(wm->ir.num_dots<=1 && szRealRealDot<=1) )
  { // If something to calculate.

    /*
    // Display values.
    printf( "nb. dot:%d, szRealRealDot:%d\n", wm->ir.num_dots, szRealRealDot ) ;

    for( i=0 ; i<4 ; i++ )
    {
      if( dot[i].visible )
        printf( "  %d:dot:%d %d\n", i, dot[i].rx, dot[i].ry ) ;
    }

    for( i=0 ; i<szRealRealDot ; i++ )
      printf( "  %d:realRealDot:%d %d\n", i, realRealDot[i][0], realRealDot[i][1] ) ;
    */

    if( szRealRealDot >= 2 )
    { // If there is enough points. No manage if there is a change points.

	    a = realRealDot[0][0] - realRealDot[1][0] ;
	    b = realRealDot[0][1] - realRealDot[1][1] ;
	    
      wm->ir.distance = sqrt(a*a + b*b) ;
      wm->ir.z = 1023.0f - wm->ir.distance ;

    }
    
    /*
    else
    { // Do something to calcul distance.
      // But something not "verify".
      // It tries to calcul with the original values get directly by the Wiimote.
      // So something not "verify". 

      int d[2][2] ;
      j = 0 ;

      for( i=0 ; i<WM_MAX_DOTS && j<2 ; i++ )
      {
	      if( dot[i].visible )
        {
		      d[j][0] = dot[i].rx ;
          d[j][1] = dot[i].ry ;
          j++ ;
        }
      }

	    a = d[0][0] - d[1][0] ;
	    b = d[0][1] - d[1][1] ;
	    
      wm->ir.distance = sqrt(a*a + b*b) ;
      wm->ir.z = 1023 - wm->ir.distance ;
    }
    */
    

    //printf( "  distance:%f, z:%f\n", wm->ir.distance, wm->ir.z ) ;
  }

  // Update yaw.
  if( wm->ir.distance != 0 )
  {
	  a = (wm->ir.ax-512) * (wm->ir.z/1024.0f) ;
    wm->orient.yaw = RAD_TO_DEGREE( atanf(a/wm->ir.z) ) ;
  }
}


/**
 *	@brief Another method to interpret IR data which base on the distance.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 */
static void interpret_ir_data3(struct wiimote_t* wm)
{
  double realDot[WM_MAX_DOTS][NBAXE]={{0,0},{0,0},{0,0},{0,0}} ;
  bool realDotIsVisible[WM_MAX_DOTS]={false,false,false,false} ;

  double **delta ; //[WM_MAX_DOTS][NBAXE]={{0,0},{0,0},{0,0},{0,0}} ;
  bool *deltaIsCalculated ; //[WM_MAX_DOTS]={false,false,false,false} ;
  double deltaFinal[NBAXE]={0,0} ;
  int nbDelta=0 ;
  //int whichAberrantDelta=0 ;
  //bool ignoreAberrantDelta=true ;

  double dist[NBPAIR]={-1,-1,-1,-1,-1,-1} ;
  double deltaDist[NBPAIR]={-1,-1,-1,-1,-1,-1} ;
  bool deltaDistIsVisible[NBPAIR]={false,false,false,false,false,false} ;
  double deltaDistFinal=0 ;
  int nbd=0 ;

  int i=0, j=0, k=0 ;
  double a=0, b=0 ;

  struct ir_dot_t* dot=wm->ir.dot ;
  wm->ir.num_dots = 0 ;
  wm->ir.ax = 0 ;
  wm->ir.ay = 0 ;

  // Allocate & init values.
  delta = (double**) malloc( sizeof(double*) * WM_MAX_DOTS ) ;
  deltaIsCalculated = (int*) malloc( sizeof(int) * WM_MAX_DOTS ) ;

  for( i=0 ; i<WM_MAX_DOTS ; i++ )
  {
    delta[i] = (double*) malloc( sizeof(double) * NBAXE ) ;
    delta[i][_X_] = 0 ;
    delta[i][_Y_] = 0 ;

    deltaIsCalculated[i] = 0 ;
  }

  // Get raw values.
	for( i=0 ; i<WM_MAX_DOTS; i++ )
	{
		if( dot[i].visible )
		{
      wm->ir.num_dots++ ;

      realDot[i][_X_] = dot[i].rx ;
      realDot[i][_Y_] = dot[i].ry ;
      realDotIsVisible[i] = true ;

      // Update absolute values.
      wm->ir.ax += dot[i].rx ;
      wm->ir.ay += dot[i].ry ;
      wm->ir.dot[i].x = dot[i].rx ;
      wm->ir.dot[i].y = dot[i].ry ;
		}
    else
    {
      wm->ir.dot[i].x = 0 ;
      wm->ir.dot[i].y = 0 ;
    }
	}

  if( wm->ir.num_dots > 0 )
  {
    wm->ir.ax = (float)wm->ir.ax / (float)wm->ir.num_dots ;
    wm->ir.ay = (float)wm->ir.ay / (float)wm->ir.num_dots ;
  }

  // Calculate distances (delta) between current and last dot.
  for( i=0 ; i<WM_MAX_DOTS ; i++ )
  {
    if( wm->ir.lastRealDotIsVisible[i]==true && realDotIsVisible[i]==true )
    {
      delta[i][_X_] = realDot[i][_X_] - wm->ir.lastRealDot[i][_X_] ;
      delta[i][_Y_] = realDot[i][_Y_] - wm->ir.lastRealDot[i][_Y_] ;
      deltaIsCalculated[i] = true ;
      nbDelta++ ;

      // Save data to watch it in an extern application.
      wm->ir.delta[i][_X_] = delta[i][_X_] ;
      wm->ir.delta[i][_Y_] = delta[i][_Y_] ;
    }
    else
    {
      deltaIsCalculated[i] = false ;

      // Save data to watch it in an extern application.
      wm->ir.delta[i][_X_] = 0 ;
      wm->ir.delta[i][_Y_] = 0 ;
    }
  }

  // Transfert the dot positions from the current variable to the last variable.
  for( i=0 ; i<WM_MAX_DOTS ; i++ )
  {
    if( realDotIsVisible[i] == true )
    {
      wm->ir.lastRealDot[i][_X_] = realDot[i][_X_] ;
      wm->ir.lastRealDot[i][_Y_] = realDot[i][_Y_] ;
      wm->ir.lastRealDotIsVisible[i] = true ;
    }
    else
    {
      wm->ir.lastRealDot[i][_X_] = 0 ;
      wm->ir.lastRealDot[i][_Y_] = 0 ;
      wm->ir.lastRealDotIsVisible[i] = false ;
    }
  }
  
  //searchAberrantValue1( wm, &nbDelta, deltaIsCalculated, delta, &whichAberrantDelta, &ignoreAberrantDelta ) ;
  searchAberrantValue2( wm, &nbDelta, deltaIsCalculated, delta ) ;

  // Save data to watch in an extern application.
  for( i=0 ; i<WM_MAX_DOTS ; i++ )
    wm->ir.tmp[0][2][i] = (double)deltaIsCalculated[i] ;


  // Calculate average of all validated delta.
  nbDelta=0 ;
  for( i=0 ; i<WM_MAX_DOTS ; i++ )
  {
    if( deltaIsCalculated[i] == true )
    {
      //if( !ignoreAberrantDelta 
      //    || (ignoreAberrantDelta && i!=whichAberrantDelta) )
      {
        deltaFinal[_X_] += delta[i][_X_] ;
        deltaFinal[_Y_] += delta[i][_Y_] ;
        nbDelta ++ ;
      }
    }
  }

  if( nbDelta != 0 )
  {
    deltaFinal[_X_] /= (double)nbDelta ;
    deltaFinal[_Y_] /= (double)nbDelta ;
  }
  else
  {
    deltaFinal[0] = 0 ;
    deltaFinal[1] = 0 ;
  }

  wm->ir.deltax = deltaFinal[_X_] ;
  wm->ir.deltay = deltaFinal[_Y_] ;

  wm->ir.x += deltaFinal[_X_] ;
  wm->ir.y += deltaFinal[_Y_] ;


  /// Distance & Yaw.

  // Calculate distance. d01, d02, d03, d12, d13, d23
  a = 0 ;
  b = 0 ;
  for( i=0, k=0 ; i<4 ; i++ )
  {
    for( j=i+1 ; j<4 ; j++ )
    {
      if( deltaIsCalculated[i]==true && deltaIsCalculated[j]==true )
      {
        a = (realDot[j][_X_]-realDot[i][_X_]) * (realDot[j][_X_]-realDot[i][_X_]) ;
        b = (realDot[j][_Y_]-realDot[i][_Y_]) * (realDot[j][_Y_]-realDot[i][_Y_]) ;
        dist[k] = sqrt( a+b ) ;
      }
      else
        dist[k] = -1 ;

      k++ ;
    }
  }
  
  // Calculate each delta, for each pair of atoms.
  for( i=0 ; i<6 ; i++ )
  {
    if( dist[i]>0 && wm->ir.lastDist[i]>0 )
    {
      deltaDist[i] = dist[i] - wm->ir.lastDist[i] ;
      deltaDistIsVisible[i] = true ;
    }
    else
      deltaDistIsVisible[i] = false ;

    wm->ir.lastDist[i] = dist[i] ;
  }

  // Calculate an average delta.
  for( i=0 ; i<6 ; i++ )
  {
    if( deltaDistIsVisible[i] )
    { 
      deltaDistFinal += deltaDist[i] ;
      nbd++ ;
    }
  }

  if( nbd > 0 )
    deltaDistFinal /= (double)nbd ;
  else
    deltaDistFinal = 0 ;

  wm->ir.deltaz = deltaDistFinal ;
  wm->ir.distance += wm->ir.deltaz ; //sqrt(a*a + b*b) ;
  wm->ir.z = 1 ;//1023.0f - wm->ir.distance ;

  // Update yaw.
  if( wm->ir.distance != 0 )
  {
	  float a = (float)(wm->ir.ax-512) * (wm->ir.z/1024.0f) ;
    wm->orient.yaw = RAD_TO_DEGREE( atanf(a/wm->ir.z) ) ;
  }


  /// Init some variable.

  // ir.state.
  if( wm->ir.num_dots >= 2 )
    wm->ir.state = 2 ;
  else
    wm->ir.state = 0 ;
  
  // Nb source detected.
  wm->ir.nb_source_detect = wm->ir.num_dots ;


  // Desalocate data.
  if( delta != NULL )
  {
    for( i=0 ; i<WM_MAX_DOTS ; i++ )
    {
      if( delta[i] != NULL )
        free( delta[i] ) ;
    }
    free( delta ) ;
    delta = NULL ;
  }

  if( deltaIsCalculated != NULL )
  {
    free( deltaIsCalculated ) ;
    deltaIsCalculated = NULL ;
  }
}

/**
 *  @brief Search aberrant values in the IR dot (v1).
 *
 *	@param nbDistCalculated_in        Number of distance between the current and last dot have been calculated.
 *	@param distIsCalculated_in        Array[WM_MAX_DOT] : The distances between the current and last dot, is it calculated ?
 *	@param dist_in                    Array[WM_MAX_DOT] : The calculated distances between the current and last dot.
 *	@param whichAberrantValue_out     The dot which is estimated aberrant.
 *	@param ignoreAberrantDist_out     Is there only one value which is estimated aberrant ?
 *
 *  To detect an aberrant value, an elapse is calculated with each dot and is compared with other.
 *  If a value is out the limit, it is evaluated aberrant+1. At the end, the value the "most aberrant"
 *  is qualified aberrant.
 */
int searchAberrantValue1( struct wiimote_t* wm,
                           int *nbDistCalculated_in, int *distIsCalculated_in, double **dist_in, 
                           int *whichAberrantValue_out, int *ignoreAberrantDist_out )
{
  double a=0, b=0 ;
  int i=0, j=0, k=0 ;
  int nbAberrantDist=0 ;
  int coeffAberrantDist[WM_MAX_DOTS]={0,0,0,0} ;
  
  // Calculate the deviation between each delta to detect an aberrant value.
  *whichAberrantValue_out = -1 ;
  *ignoreAberrantDist_out = false ;

  if( wm->ir.num_dots>=2 && *nbDistCalculated_in>=2 )
  {
    for( i=0 ; i<WM_MAX_DOTS ; i++ )
    {
      if( distIsCalculated_in[i] == true )
      {
        // Get the authorized elapse.
        if( fabs(dist_in[i][_X_]) > 20 )
          a = fabs( dist_in[i][_X_]*0.5 ) ;
        else
          a = 10 ;

        if( fabs(dist_in[i][_Y_]) > 15 )
          b = fabs( dist_in[i][_Y_]*0.5 ) ;
        else
          b = 10 ;

        // Search aberrant values.
        for( j=0 ; j<WM_MAX_DOTS ; j++ )
        {
          if( i!=j && distIsCalculated_in[j]==true )
          {
            // Is'nt it authorized ?
            if( !( 
                  ((dist_in[i][_X_]-a) < dist_in[j][_X_] && dist_in[j][_X_] < (dist_in[i][_X_]+a))
                  && ((dist_in[i][_Y_]-b) < dist_in[j][_Y_] && dist_in[j][_Y_] < (dist_in[i][_Y_]+b))
                  ))
            {
              coeffAberrantDist[j] ++ ;
              k++ ;
            }
          }
        }
      
        // If all other values are aberrant, I am aberrant!
        // Only if the 3-4 dots are visible.
        if( k == (wm->ir.nb_source_detect-1) )
        {
          // I am aberrant.
          coeffAberrantDist[i]++ ;

          // Not others.
          for( j=0 ; j<WM_MAX_DOTS ; j++ )
          {
            if( j != i )
              coeffAberrantDist[j]-- ;
          }
        }

        k = 0 ;
      }
    }
  
    // Get the 1st aberrant value.
    *whichAberrantValue_out = -1 ;
    nbAberrantDist = 0 ;
    i=0 ; k=0 ;
    while( i<WM_MAX_DOTS && k==0 )
    {
      if( coeffAberrantDist[i] > 0 )
      {
        *whichAberrantValue_out = i ;
        nbAberrantDist = 1 ;
        k = 1 ;
      }
      i++ ;
    }

    // Get the aberrant value.
    if( nbAberrantDist > 0 )
    {      
      for( i=*whichAberrantValue_out+1 ; i<WM_MAX_DOTS ; i++ )
      {
        if( coeffAberrantDist[*whichAberrantValue_out] <= coeffAberrantDist[i] )
        {
          if( coeffAberrantDist[*whichAberrantValue_out] == coeffAberrantDist[i] )
          {
            (nbAberrantDist)++ ;
          }
          else
          {
            *whichAberrantValue_out = i ;
            nbAberrantDist = 1 ;
          }
        }
      }
    }

    // If all aberrant values are egual, there are no aberrant.
    //if( nbAberrantDist == wm->ir.nb_source_detect )
      //nbAberrantDist = 0 ;
  }

  // Save data to watch in an extern application.
  for( i=0 ; i<WM_MAX_DOTS ; i++ )
    wm->ir.tmp[0][1][i] = (double)coeffAberrantDist[i] ;

  // What strategie for the aberrant value.
  if( nbAberrantDist == 0 )
  { // No abberant value, so no problem.

    *whichAberrantValue_out = -1 ;
    *ignoreAberrantDist_out = false ;
    wm->ir.tmp[0][0][0] = -1 ;
  }
  else if( nbAberrantDist == 1 )
  { // Ignore the only aberrant value.

    *ignoreAberrantDist_out = true ;
    wm->ir.tmp[0][0][0] = *whichAberrantValue_out ;
  }
  else if( nbAberrantDist > 1 )
  { // Too many aberrant value, do nothing.

    *whichAberrantValue_out = -1 ;
    *ignoreAberrantDist_out = false ;
    wm->ir.tmp[0][0][0] = -2 ;
  }
  else
  { // No sé. Impossible case.

    *whichAberrantValue_out = -1 ;
    *ignoreAberrantDist_out = false ;
    wm->ir.tmp[0][0][0] = -3 ;
  }

  return nbAberrantDist ;
}


/**
 *  @brief Search aberrant values in the IR dot (v2).
 *
 *	@param nbDistCalculated_in   Number of distance between the current and last dot have been calculated.
 *	@param distIsCalculated_in   Array[WM_MAX_DOT] : The distances between the current and last dot, is it calculated ?
 *	@param dist_in               Array[WM_MAX_DOT] : The calculated distances between the current and last dot.
 *
 *  To detect an aberrant value, a standard deviation is calculated, then other data are filtered
 *  from this std deviation.
 *  (distIsCalculated_in) variable is directly changed to avoid the final average calcul.
 */
void searchAberrantValue2( struct wiimote_t* wm,
                           int *nbDistCalculated_in, int *distIsCalculated_in, double **dist_in )
{
  const int LIMITZP=3 ; // Limit to detect a zone precision.
  bool datasetAberrant=false ;
  bool valueAberrant=false ;

  double stdDeviationX=0, stdDeviationY=0 ;
  double stdDeviationRoundX=0, stdDeviationRoundY=0 ;
  double predictedStdDeviationX=0, predictedStdDeviationY=0 ;
  double averageX=0, averageY=0 ;

  double a=0, b=0 ;
  int i=0 ;

  if( (*nbDistCalculated_in) == 0 )
  {
    wm->ir.isInPrecisionMode = 0 ;
  }
  else if( (*nbDistCalculated_in)==1 || (*nbDistCalculated_in)==2 )
  {
    calculateAverage( nbDistCalculated_in, distIsCalculated_in, dist_in, &averageX, &averageY ) ;

    if( fabs(averageX)<LIMITZP && fabs(averageY)<LIMITZP )
      wm->ir.isInPrecisionMode = 1 ;
    else
      wm->ir.isInPrecisionMode = 0 ;
  }
  else if( (*nbDistCalculated_in) > 2 )
  {
    int repeat1=1 ;

    do
    {
      repeat1-- ;
      valueAberrant = false ;

      calculateAverage( nbDistCalculated_in, distIsCalculated_in, dist_in, &averageX, &averageY ) ;
      calculateStdDeviation( nbDistCalculated_in, distIsCalculated_in, dist_in, &averageX, &averageY, &stdDeviationX, &stdDeviationY ) ;

      if( fabs(averageX)<=LIMITZP && fabs(averageY)<=LIMITZP
          && stdDeviationX<=LIMITZP && stdDeviationY<=LIMITZP )
      { // If precision zone (std deviation < 3) (3 : experimental value)

        // Just go to the next calculation "Search aberrant value".
        wm->ir.isInPrecisionMode = 1 ;
      }
      else if( repeat1 >= 0 )
      { // In this case, a prediction of the std deviation is realized to detect 
        // if the dataset is aberrant.

        wm->ir.isInPrecisionMode = 0 ;
        calculateStdDeviationPredited( &averageX, &averageY, &predictedStdDeviationX, &predictedStdDeviationY ) ;

        // Compared (%) with the calculated std dev.
        if( stdDeviationX!=0 && stdDeviationY!=0 )
        {
          a = fabs(predictedStdDeviationX-stdDeviationX) / stdDeviationX ;
          b = fabs(predictedStdDeviationY-stdDeviationY) / stdDeviationY ;
        }
        else
        {
          a = 0 ;
          b = 0 ;
        }
        
        if( a>50.0 || b>50.0 )
        { // If the calculated std deviation is greater than 50% of the predicted std deviation,
          // the data is "deleted".
          datasetAberrant = true ;
        }
      }

      // Update (distIsCalculated_in), if the distance can be used to calculated the final average.
      if( datasetAberrant == true )
      { // "Delete" the dataset.

        for( i=0 ; i<WM_MAX_DOTS ; i++ )
        {
          if( distIsCalculated_in[i] == true )
          {
            distIsCalculated_in[i] = false ;
            (*nbDistCalculated_in) -- ;
          }
        }
      }
      else
      { // Search aberrant value.

        // Round Up the std deviation.
        stdDeviationRoundX = ceil(stdDeviationX) + 3.0 ;
        stdDeviationRoundY = ceil(stdDeviationY) + 3.0 ;

        for( i=0 ; i<WM_MAX_DOTS ; i++ )
        {
          if( distIsCalculated_in[i] == true )
          {
            a = fabs( fabs(dist_in[i][_X_]) - fabs(averageX) ) ;
            b = fabs( fabs(dist_in[i][_Y_]) - fabs(averageY) ) ;

            // If aberrant value, do not take it in the final average.
            if( !( a<=stdDeviationRoundX && b<=stdDeviationRoundY ) )
            {
              valueAberrant = true ;

              distIsCalculated_in[i] = false ;
              (*nbDistCalculated_in) -- ;
              
              wm->ir.tmp[0][0][0] = i ;
            }
          }
        }
      }
    }while( repeat1>=0 && valueAberrant==true ) ;
  }
  
  if( datasetAberrant == true )
    wm->ir.tmp[0][0][0] = -2 ;
  if( valueAberrant == true )
  {
    // wm->ir.tmp[0][0][0] >= 0
  }
  else
    wm->ir.tmp[0][0][0] = -1 ;
}

/**
 *	@brief Calculate the average of the ir dot position.
 *
 *	@param nbDistCalculated_in   Number of distance between the current and last dot have been calculated.
 *	@param distIsCalculated_in   Array[WM_MAX_DOT] : The distances between the current and last dot, is it calculated ?
 *	@param dist_in               Array[WM_MAX_DOT] : The calculated distances between the current and last dot.
 *	@param averageX_out		       Average in X-axis.
 *	@param averageY_out		       Average in Y-axis.
 */
void calculateAverage( int *nbDistCalculated_in, int *distIsCalculated_in, double **dist_in,
                       double *averageX_out, double *averageY_out )
{
  int i=0 ;
  (*averageX_out) = 0 ;
  (*averageY_out) = 0 ;
  
  // Calculated the average.
  for( i=0 ; i<WM_MAX_DOTS ; i++ )
  {
    if( distIsCalculated_in[i] == true )
    {
      (*averageX_out) += dist_in[i][_X_] ;
      (*averageY_out) += dist_in[i][_Y_] ;
    }
  }
  (*averageX_out) /= (double)(*nbDistCalculated_in) ;
  (*averageY_out) /= (double)(*nbDistCalculated_in) ;
}


/**
 *	@brief Calculate the standard deviation ((fr:)ecart type) of the ir dot position.
 *
 *	@param nbDistCalculated_in   Number of distance between the current and last dot have been calculated.
 *	@param distIsCalculated_in   Array[WM_MAX_DOT] : The distances between the current and last dot, is it calculated ?
 *	@param dist_in               Array[WM_MAX_DOT] : The calculated distances between the current and last dot.
 *	@param averageX_in		       Average on X-axis.
 *	@param averageY_in		       Average on Y-axis.
 *	@param averageX_out		       Standard deviation on X-axis.
 *	@param averageY_out		       Standard deviation on Y-axis.
 */
void calculateStdDeviation( int *nbDistCalculated_in, int *distIsCalculated_in, double **dist_in, double *averageX_in, double *averageY_in,
                            double *stdDeviationX_out, double *stdDeviationY_out )
{
  int i=0 ;
  (*stdDeviationX_out) = 0 ;
  (*stdDeviationY_out) = 0 ;
  
  // Calculated standard deviation.
  for( i=0 ; i<WM_MAX_DOTS ; i++ )
  {
    if( distIsCalculated_in[i] == true )
    {
      (*stdDeviationX_out) += ((dist_in[i][_X_] - (*averageX_in)) * (dist_in[i][_X_] - (*averageX_in))) ;
      (*stdDeviationY_out) += ((dist_in[i][_Y_] - (*averageY_in)) * (dist_in[i][_Y_] - (*averageY_in))) ;
    }
  }

  (*stdDeviationX_out) *= (1.0 / (double)(*nbDistCalculated_in)) ;
  (*stdDeviationY_out) *= (1.0 / (double)(*nbDistCalculated_in)) ;
  (*stdDeviationX_out) = sqrt((*stdDeviationX_out)) ;
  (*stdDeviationY_out) = sqrt((*stdDeviationY_out)) ;
}


/**
 *	@brief Calculate the predicted standard deviation of the ir dot position.
 *
 *	@param averageX_in		       Average on X-axis.
 *	@param averageY_in		       Average on Y-axis.
 *	@param stdDevPredictedX_out	 Predicted standard deviation on X-axis.
 *	@param stdDevPredictedY_out  Predicted standard deviation on Y-axis.
 *
 * Experimentally, a linear regression has been calculated with a dataset : std dev / |avg| .
 * The calculated linear regression is : (predicted std dev) = 0.07333454 . (avg) + 1.60458
 * ((fr:)regression lineaire : droite "moyennant" toutes les valeurs sur une droite de la forme a.x+b).
 */
void calculateStdDeviationPredited( double *averageX_in, double *averageY_in, 
                                    double *stdDevPredictedX_out, double *stdDevPredictedY_out )
{
  // Predicted std dev.
  (*stdDevPredictedX_out) = 0.07333154 * (*averageX_in) + 1.60458 ;
  (*stdDevPredictedY_out) = 0.07333154 * (*averageY_in) + 1.60458 ;
}



/**
 *	@brief Fix the rotation of the IR dots.
 *
 *	@param dot		An array of 4 ir_dot_t objects.
 *	@param ang		The roll angle to correct by (-180, 180)
 *
 *	If there is roll then the dots are rotated
 *	around the origin and give a false cursor
 *	position. Correct for the roll.
 *
 *	If the accelerometer is off then obviously
 *	this will not do anything and the cursor
 *	position may be inaccurate.
 */
static void fix_rotated_ir_dots(struct ir_dot_t* dot, float ang) {
	float s, c;
	int x, y;
	int i;

	if (!ang) {
		for (i = 0; i < 4; ++i) {
			dot[i].x = dot[i].rx;
			dot[i].y = dot[i].ry;
		}
		return;
	}

	s = sin(DEGREE_TO_RAD(ang));
	c = cos(DEGREE_TO_RAD(ang));

	/*
	 *	[ cos(theta)  -sin(theta) ][ ir->rx ]
	 *	[ sin(theta)  cos(theta)  ][ ir->ry ]
	 */

	for (i = 0; i < 4; ++i) {
		if (!dot[i].visible)
			continue;

		x = dot[i].rx - (1024/2);
		y = dot[i].ry - (768/2);

		dot[i].x = (c * x) + (-s * y);
		dot[i].y = (s * x) + (c * y);

		dot[i].x += (1024/2);
		dot[i].y += (768/2);
	}
}


/**
 *	@brief Average IR dots.
 *
 *	@param dot		An array of 4 ir_dot_t objects.
 *	@param x		[out] Average X
 *	@param y		[out] Average Y
 */
static void get_ir_dot_avg(struct ir_dot_t* dot, int* x, int* y) {
	int vis = 0, i = 0;

	*x = 0;
	*y = 0;

	for (; i < 4; ++i) {
		if (dot[i].visible) {
			*x += dot[i].x;
			*y += dot[i].y;
			++vis;
		}
	}

	*x /= vis;
	*y /= vis;
}


/**
 *	@brief Reorder the IR dots.
 *
 *	@param dot		An array of 4 ir_dot_t objects.
 */
static void reorder_ir_dots(struct ir_dot_t* dot) {
	int i, j, order;

	/* reset the dot ordering */
	for (i = 0; i < 4; ++i)
		dot[i].order = 0;

	for (order = 1; order < 5; ++order) {
		i = 0;

		for (; !dot[i].visible || dot[i].order; ++i)
		if (i > 4)
			return;

		for (j = 0; j < 4; ++j) {
			if (dot[j].visible && !dot[j].order && (dot[j].x < dot[i].x))
				i = j;
		}

		dot[i].order = order;
	}
}


/**
 *	@brief Calculate the distance between the first 2 visible IR dots.
 *
 *	@param dot		An array of 4 ir_dot_t objects.
 */
static float ir_distance(struct ir_dot_t* dot) {
	int i1, i2;
	int xd, yd;

	for (i1 = 0; i1 < 4; ++i1)
		if (dot[i1].visible)
			break;
	if (i1 == 4)
		return 0.0f;

	for (i2 = i1+1; i2 < 4; ++i2)
		if (dot[i2].visible)
			break;
	if (i2 == 4)
		return 0.0f;

	xd = dot[i2].x - dot[i1].x;
	yd = dot[i2].y - dot[i1].y;

	return sqrt(xd*xd + yd*yd);
}


/**
 *	@brief Correct for the IR bounding box.
 *
 *	@param x		[out] The current X, it will be updated if valid.
 *	@param y		[out] The current Y, it will be updated if valid.
 *	@param aspect	Aspect ratio of the screen.
 *	@param offset_x	The X offset of the bounding box.
 *	@param offset_y	The Y offset of the bounding box.
 *
 *	@return Returns 1 if the point is valid and was updated.
 *
 *	Nintendo was smart with this bit. They sacrifice a little
 *	precision for a big increase in usability.
 */
static int ir_correct_for_bounds(int* x, int* y, enum aspect_t aspect, int offset_x, int offset_y) {
	int x0, y0;
	int xs, ys;

	if (aspect == WIIUSE_ASPECT_16_9) {
		xs = WM_ASPECT_16_9_X;
		ys = WM_ASPECT_16_9_Y;
	} else {
		xs = WM_ASPECT_4_3_X;
		ys = WM_ASPECT_4_3_Y;
	}

	x0 = ((1024 - xs) / 2) + offset_x;
	y0 = ((768 - ys) / 2) + offset_y;

	if ((*x >= x0)
		&& (*x <= (x0 + xs))
		&& (*y >= y0)
		&& (*y <= (y0 + ys)))
	{
		*x -= offset_x;
		*y -= offset_y;

		return 1;
	}

	return 0;
}


/**
 *	@brief Interpolate the point to the user defined virtual screen resolution.
 */
static void ir_convert_to_vres(int* x, int* y, enum aspect_t aspect, int vx, int vy) {
	int xs, ys;

	if (aspect == WIIUSE_ASPECT_16_9) {
		xs = WM_ASPECT_16_9_X;
		ys = WM_ASPECT_16_9_Y;
	} else {
		xs = WM_ASPECT_4_3_X;
		ys = WM_ASPECT_4_3_Y;
	}

	*x -= ((1024-xs)/2);
	*y -= ((768-ys)/2);

	*x = (*x / (float)xs) * vx;
	*y = (*y / (float)ys) * vy;
}


/**
 *	@brief Calculate yaw given the IR data.
 *
 *	@param ir	IR data structure.
 */
float calc_yaw(struct ir_t* ir) {
	float x;

	x = ir->ax - 512;
	x = x * (ir->z / 1024.0f);

	return RAD_TO_DEGREE( atanf(x / ir->z) );
}
