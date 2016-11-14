clc;clear all;close;
    
    [rho,theta,phi,alpha,beta,vl,vr,state,ms,speed,pwm_speed]=textread('control_output_m.txt','%f%f%f%f%f%f%f%f%f%f%f',...
        'headerlines',0);
    rho = rho/100;
    
% windowSize = 30;
% b = (1/windowSize)*ones(1,windowSize);
% a = 1;
% speed_filter = filter(b,a,speed);    
  
speed_filter = medfilt1(speed,63);

    
figure(1);  
%plot(1:size(rho,1),rho*100, 1:size(theta,1),theta*180/pi, 1:size(phi,1),phi*180/pi,...
%    1:size(alpha,1),alpha*180/pi, 1:size(beta,1),beta*180/pi, 1:size(state,1),state*10,'LineWidth',1.5); hold on;

subplot(2,3,1); plot(1:size(rho,1),rho*100);hold on;
title('\fontsize{14} \fontname{Times New Roman} rho trajectory');
xlabel('\fontsize{14} \fontname{Times New Roman} s/30');
ylabel('\fontsize{14} \fontname{Times New Roman} rho(m)');
axis([0,size(rho,1),-inf,inf]);grid on;
%axis auto;grid on;

subplot(2,3,2); plot(1:size(phi,1),phi*180/pi);hold on;
title('\fontsize{14} \fontname{Times New Roman} phi trajectory');
xlabel('\fontsize{14} \fontname{Times New Roman} s/30');
ylabel('\fontsize{14} \fontname{Times New Roman} phi(degree)');
axis([0,size(phi,1),-180,180]);grid on;

subplot(2,3,3); plot(1:size(alpha,1),alpha*180/pi);hold on;
title('\fontsize{14} \fontname{Times New Roman} alpha trajectory');
xlabel('\fontsize{14} \fontname{Times New Roman} s/30');
ylabel('\fontsize{14} \fontname{Times New Roman} alpha(degree)');
axis([0,size(alpha,1),-180,180]);grid on;

subplot(2,3,4); plot(1:size(beta,1),beta*180/pi);hold on;
title('\fontsize{14} \fontname{Times New Roman} beta trajectory');
xlabel('\fontsize{14} \fontname{Times New Roman} s/30');
ylabel('\fontsize{14} \fontname{Times New Roman} beta(degree)');
axis([0,size(beta,1),-180,180]);grid on;

subplot(2,3,5); plot(1:size(speed,1),speed_filter);hold on;
title('\fontsize{14} \fontname{Times New Roman} speed trajectory');
xlabel('\fontsize{14} \fontname{Times New Roman} s/30');
ylabel('\fontsize{14} \fontname{Times New Roman} speed(m/s)');
axis([0,size(speed,1),-inf,1.5]);grid on;

subplot(2,3,6); plot(1:size(pwm_speed,1),pwm_speed);hold on;
title('\fontsize{14} \fontname{Times New Roman} PWM trajectory');
xlabel('\fontsize{14} \fontname{Times New Roman} s/30');
ylabel('\fontsize{14} \fontname{Times New Roman} PWM');
axis([0,size(pwm_speed,1),-inf,inf]);grid on;

% subplot(2,3,5); plot(1:size(vl,1),vl);hold on;
% title('\fontsize{14} \fontname{Times New Roman} vl trajectory');
% xlabel('\fontsize{14} \fontname{Times New Roman} s/30');
% ylabel('\fontsize{14} \fontname{Times New Roman} vl(speed)');
% axis([0,size(vl,1),-3,3]);grid on;
% 
% subplot(2,3,6); plot(1:size(vr,1),vr);hold on;
% title('\fontsize{14} \fontname{Times New Roman} vr trajectory');
% xlabel('\fontsize{14} \fontname{Times New Roman} s/30');
% ylabel('\fontsize{14} \fontname{Times New Roman} vr(speed)');
% axis([0,size(vr,1),-3,3]);grid on;



% plot([1 size(rho,1)],[180 180],'--');
% plot([1 size(rho,1)],[-180 -180],'--');
% plot([1 size(rho,1)],[0 0],'--');
% legend('rho','theta','phi','alpha','beta','state');


% figure(2);
% plot(vl); hold on;
% plot(vr); hold on;
% axis([0,size(rho,1),-10,10]);



 

   