void chooseDirection(){
  /* at end direction integer will contain information corresponding to a direction
  */
    if(pos.y>0.1 && pos.z>0.1 && abs(pos.y-pos.z)<0.15){
      Serial.println("left and up");
      direction = 4;
    }else if(pos.y<-0.1 && pos.z>0.1 && abs(abs(pos.y)-pos.z)<0.15){
      Serial.println("right and up");
      direction = 5;
    } else if(pos.y>0.1 && pos.z<-0.1 && abs(pos.y-abs(pos.z))<0.15){
      Serial.println("left and down");
      direction = 7;
    } else if(pos.y<-0.1 && pos.z<-0.1 && abs(pos.y-pos.z)<0.15){
      Serial.println("right and down");
      direction = 6;
    }else if(abs(pos.y)>abs(pos.z) && pos.y>0.1){
      Serial.println("left");
      direction = 2;
    }else if (abs(pos.y)>abs(pos.z) && pos.y<-0.1){
      Serial.println("right");
      direction = 3;
    }else if(pos.z>0.1){
      Serial.println("up");
      direction = 0;
    }else if (pos.z<-0.1){
      Serial.println("down");
      direction = 1;
    } else{
      Serial.println("no movement");
      direction = 8;
    }



   sprintf(output, "z: %4.6f, y: %4.6f", pos.z, pos.y);
  Serial.println(output);
  //Serial.println(direction);
}

void calcAccel(){
  //sets accel_real to correct direction of motion removing gravity
  vec3 curr_acc = imu.accel();
  vec3 accel_rotated = rotateVec(&curr_acc);
  acc.x =0;
  acc.y = accel_rotated.y - gravity.y;
  acc.z = accel_rotated.z - gravity.z;
}

void calcPos(){
  // integrates accel real to get pos_real
  integrateVec(&acc, &vel);
  integrateVec(&vel, &pos);
}

void getAngle(){
  //integrates angular velocity to get angle
  ang+= imu.poll().gyro() * (DT / 1000.0);
}

void resetOrientation(){
  //calculate current angle based on gravity, assume not moving
  vec3 curr_acc = imu.poll().get_acc();
  ang.x = asin(curr_acc.y/sqrt(curr_acc.y*curr_acc.y+curr_acc.z*curr_acc.z));
  ang.y = asin(curr_acc.x/sqrt(curr_acc.x*curr_acc.x+curr_acc.z*curr_acc.z));

}

vec3 rotateVec(vec3 * vec){
  // rotates vector around x axis based on the angle of the remote
  vec3 vec_new;
  vec_new.x = cos(ang.y)*vec->x - sin(ang.y)* vec->z;
  vec_new.z = sin(ang.y)*vec->x + cos(ang.y)* vec->z;

  vec_new.y = cos(ang.x)*vec->y - sin(ang.x)* vec_new.z;
  vec_new.z = sin(ang.x)*vec->y + cos(ang.x)* vec_new.z;

  return vec_new;
}

void integrateVec(vec3 * vec, vec3 * total){
  // integrates vector struct
  total->x = total->x+ 0.001*DT*vec->x;
  total->y = total->y+ 0.001*DT*vec->y;
  total->z = total->z+ 0.001*DT*vec->z;
}