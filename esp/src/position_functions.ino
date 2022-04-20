void chooseDirection(){
  memset(direction, '0', 4);
  //if (abs(pos_real.y)>abs(pos_real.z)){
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
  //} else {
    if(pos_real.z>0.01){
      direction[2]='1';
      direction[3]='0';
      count++;
    }else if (pos_real.z<-0.01){
      direction[2]='0';
      direction[3]='1';
      count++;
    }

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

  //}
  sprintf(output, "z: %4.6f, y: %4.6f", pos_real.z, pos_real.y);
  Serial.println(output);
  Serial.println(direction);


}

void calcAccel(){
  struct vector accel = getAccel();
  struct vector accel_rotated = rotateVec(accel);

  accel_real.y = accel_rotated.y-gravity.y;
  accel_real.z = accel_rotated.z - gravity.z;
}

void calcPos(){
  integrateVec(accel_real, &vel_real);
  integrateVec(vel_real, &pos_real);
}

void getAngle(){
  imu.readGyroData(imu.gyroCount);
  ang_vel = imu.gyroCount[0] * imu.gRes;
  ang_pos= ang_pos+ 0.001*DT*ang_vel ;
}

void initializeOrientation(){
  ang_pos = 0;
  imu.readAccelData(imu.accelCount);
  gravity.y = imu.accelCount[1] * imu.aRes; 
  gravity.z = imu.accelCount[2] * imu.aRes;
}

void resetOrientation(){
  struct vector accel = getAccel();
  ang_pos = 180.0/3.1415*asin(accel.y/sqrt(accel.y*accel.y+accel.z*accel.z));
}

void calculateDrift(){
  pos_real.y=0; 
  pos_real.z=0; 
  vel_real.z =0;
  vel_real.y = 0;
  ang_pos=0;

  for (int i=0;i<10; i++){
    getAngle();
    calcAccel();
    calcPos();
    while(millis()-primary_timer<DT);
    primary_timer=millis();
  }
  y_drift = (pos_real.y)/10.0;
  z_drift = (pos_real.z)/10.0;
  ang_drift = (ang_pos)/10.0;
  pos_real.y=0; 
  pos_real.z=0; 
  vel_real.z =0;
  vel_real.y = 0;
  ang_pos=0;
    
}

struct vector getAccel(){
  imu.readAccelData(imu.accelCount);
  struct vector accel;
  accel.y = imu.accelCount[1] * imu.aRes; 
  accel.z = imu.accelCount[2] * imu.aRes;
  return accel;
}

struct vector rotateVec(struct vector vec){
  struct vector vec_new;
  vec_new.y = cos(ang_pos*3.14/180)*vec.y - sin(ang_pos*3.14/180)* vec.z;
  vec_new.z = sin(ang_pos*3.14/180)*vec.y + cos(ang_pos*3.14/180)* vec.z;
  return vec_new;
}

void integrateVec(struct vector vec, struct vector* total){
  total->y = total->y+ 0.001*DT*vec.y;
  total->z = total->z+ 0.001*DT*vec.z;
}