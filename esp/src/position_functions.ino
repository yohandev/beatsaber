void chooseDirection(){
  /* at end direction array will contain information corresponding to a direction
  0000: no movement, 1000: left, 0100: right, 0010: up, 0001: down, 
  if two 1s are present, there was likely movement in two directions
  */
  memset(direction, '0', 4);
    int count = 0;    
    if(pos_real.y>0.01){
      direction[0]='1';
      direction[1]='0';
      count++;
    }else if (pos_real.y<-0.01){
      direction[0]='0';
      direction[1]='1';
      count++;
    }
    if(pos_real.z>0.01){
      direction[2]='1';
      direction[3]='0';
      count++;
    }else if (pos_real.z<-0.01){
      direction[2]='0';
      direction[3]='1';
      count++;
    }

  //process for nice serial monitor output
  if (direction[0]=='1'){
    Serial.print("left");
  } else if (direction[1]=='1'){
    Serial.print("right");
  } 
  if ((direction[0]=='1' || direction[1]=='1') && count==1){
    Serial.println();
  } else if (count==2){
    Serial.print(" or ");
  }
  if (direction[2]=='1'){
    Serial.println("up");
  } else if (direction[3]=='1'){
    Serial.println("down");
  } 
  if(count==0){
Serial.println("no movement"); 
  }

   sprintf(output, "z: %4.6f, y: %4.6f", pos_real.z, pos_real.y);
  Serial.println(output);
  Serial.println(direction);


}

void calcAccel(){
  //sets accel_real to correct direction of motion removing gravity
  struct vector accel = getAccel();
  struct vector accel_rotated = rotateVec(accel);
  accel_real.x =0;
  accel_real.y = accel_rotated.y-gravity.y;
  accel_real.z = accel_rotated.z - gravity.z;
}

void calcPos(){
  // integrates accel real to get pos_real
  integrateVec(accel_real, &vel_real);
  integrateVec(vel_real, &pos_real);
}

void getAngle(){
  //integrates angular velocity to get angle
  imu.readGyroData(imu.gyroCount);
  ang_velx = imu.gyroCount[0] * imu.gRes;
  ang_x= ang_x+ 0.001*DT*ang_velx ;
  ang_vely = imu.gyroCount[1] * imu.gRes;
  ang_y= ang_y+ 0.001*DT*ang_vely ;
}

void initializeOrientation(){
  //set angle to 0 and set gravity accordingly
  ang_x = 0;
  ang_y = 0;
  imu.readAccelData(imu.accelCount);
  gravity.y = imu.accelCount[1] * imu.aRes; 
  gravity.z = imu.accelCount[2] * imu.aRes;
}

void resetOrientation(){
  //calculate current angle based on gravity, assume not moving
  imu.readAccelData(imu.accelCount);
  float x = imu.accelCount[0];
  float y = imu.accelCount[1];
  float z = imu.accelCount[2];
  ang_x = 180.0/3.1415*asin(y/sqrt(y*y+z*z));
  ang_y = 180.0/3.1415*asin(x/sqrt(x*x+z*z));

}

void calculateDrift(){
  //over 10 loop cycles worth of time, calculate the drift in each measurement
  pos_real.y=0; 
  pos_real.z=0; 
  vel_real.z =0;
  vel_real.y = 0;
  ang_x=0;
  ang_y=0;

  for (int i=0;i<10; i++){
    getAngle();
    calcAccel();
    calcPos();
    while(millis()-primary_timer<DT);
    primary_timer=millis();
  }
  y_drift = (pos_real.y)/10.0;
  z_drift = (pos_real.z)/10.0;
  ang_driftx = (ang_x)/10.0;
  ang_drifty = (ang_y)/10.0;
  pos_real.y=0; 
  pos_real.z=0; 
  vel_real.z =0;
  vel_real.y = 0;
  ang_x=0;
  ang_y = 0;
    
}

struct vector getAccel(){
  // returns acceleration as vector struct
  imu.readAccelData(imu.accelCount);
  struct vector accel;
  accel.x = imu.accelCount[1] * imu.aRes; 
  accel.y = imu.accelCount[1] * imu.aRes; 
  accel.z = imu.accelCount[2] * imu.aRes;
  return accel;
}

struct vector rotateVec(struct vector vec){
  // rotates vector around x axis based on the angle of the remote
  struct vector vec_new;
  vec_new.x = cos(ang_y*3.14/180)*vec.x - sin(ang_y*3.14/180)* vec.z;
  vec_new.z = sin(ang_y*3.14/180)*vec.x + cos(ang_y*3.14/180)* vec.z;

  vec_new.y = cos(ang_x*3.14/180)*vec.y - sin(ang_x*3.14/180)* vec.z;
  vec_new.z = sin(ang_x*3.14/180)*vec.y + cos(ang_x*3.14/180)* vec.z;

  return vec_new;
}

void integrateVec(struct vector vec, struct vector* total){
  // integrates vector struct
  total->x = total->x+ 0.001*DT*vec.x;
  total->y = total->y+ 0.001*DT*vec.y;
  total->z = total->z+ 0.001*DT*vec.z;
}