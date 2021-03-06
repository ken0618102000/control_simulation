clc;clear all;close;
    
    [angle1,x1,y1,time]=textread('control_pos_m.txt','%f%f%f%f','headerlines',0);
 %   angle1=angle1+pi/2;
%     x1=x1*100;
%     y1=y1*100;
    
    
   halfL = 0.21/2; halfW =0.21/2;     %%%   original robot posture (corner)
   cornerR = [halfL -halfL -halfL halfL halfL;
   halfW halfW -halfW -halfW halfW];
   arrowR = [1.5*halfL 0]';  % robot orientation arrow

    for i=0:5:size(x1,1)
       x2(1+ i/5)=x1(i+1);
       y2(1+ i/5)=y1(i+1);
    end
    plot(x2,y2,'r','LineWidth',2);hold on;
%     plot(x1,y1,'r','LineWidth',1);hold on;
    
   for i=1:50:size(x1,1)
       %%if((((x(i+1)-x(i))^2+y(i+1)-y(i))^2) < 6) 
        
        
           angle1(i) = wrapToPi( angle1(i) );
           rotationM = [cos(angle1(i))  -sin(angle1(i)) ; sin(angle1(i))  cos(angle1(i))];
           rotated_cornerR = rotationM * cornerR + [x1(i) 0;0 y1(i)]*ones(size(cornerR));
           rotated_arrowR = rotationM * arrowR + [x1(i) y1(i)]';    
           plot(rotated_cornerR(1,:), rotated_cornerR(2,:),'g-','LineWidth',2.5); hold on;
           plot([x1(i) rotated_arrowR(1)],[y1(i) rotated_arrowR(2)],'b-','LineWidth',1.5); hold on;
           if i==1;
                plot(rotated_cornerR(1,:), rotated_cornerR(2,:),'r-','LineWidth',2.5); hold on;
                plot([x1(i) rotated_arrowR(1)],[y1(i) rotated_arrowR(2)],'r-','LineWidth',1.5); hold on;
           end
      %% end
   end
  angle1(size(x1,1)) = wrapToPi( angle1(size(x1,1)) );
  rotationM = [cos(angle1(size(x1,1)))  -sin(angle1(size(x1,1))) ; sin(angle1(size(x1,1)))  cos(angle1(size(x1,1)))];
  rotated_cornerR = rotationM * cornerR + [x1(size(x1,1)) 0;0 y1(size(x1,1))]*ones(size(cornerR));
  rotated_arrowR = rotationM * arrowR + [x1(size(x1,1)) y1(size(x1,1))]';  
  plot(rotated_cornerR(1,:), rotated_cornerR(2,:),'--black','LineWidth',2.5); hold on;
  plot([x1(size(x1,1)) rotated_arrowR(1)],[y1(size(x1,1)) rotated_arrowR(2)],'--black','LineWidth',1.5); hold on;
  
title('\fontsize{14} \fontname{Times New Roman} trajectory');
xlabel('\fontsize{14} \fontname{Times New Roman} m');
ylabel('\fontsize{16} \fontname{Times New Roman} m');
 % axis([-0.5,2.5,-2.5,0.5]);grid on;
   
   grid on;
 

   