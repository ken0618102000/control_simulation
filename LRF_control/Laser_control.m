clc;clear all;close;
    
    [rho,theta,phi,alpha,beta,vl,vr,state]=textread('control_output_m.txt','%f%f%f%f%f%f%f%f',...
        'headerlines',0);
    rho = rho/100;
    
figure(1);  
%plot(1:size(rho,1),rho*100, 1:size(theta,1),theta*180/pi, 1:size(phi,1),phi*180/pi,...
%    1:size(alpha,1),alpha*180/pi, 1:size(beta,1),beta*180/pi, 1:size(state,1),state*10,'LineWidth',1.5); hold on;

subplot(2,3,1); plot(1:size(rho,1),rho*100);hold on;
title('\fontsize{14} \fontname{Times New Roman} rho trajectory');
xlabel('\fontsize{14} \fontname{Times New Roman} s/10');
ylabel('\fontsize{14} \fontname{Times New Roman} rho(m)');
axis([0,size(rho,1),-inf,inf]);
grid on;

subplot(2,3,2); plot(1:size(phi,1),phi*180/pi);hold on;
title('\fontsize{14} \fontname{Times New Roman} phi trajectory');
xlabel('\fontsize{14} \fontname{Times New Roman} s/10');
ylabel('\fontsize{14} \fontname{Times New Roman} phi(degree)');
axis([0,size(rho,1),-inf,inf]);
grid on;

subplot(2,3,3); plot(1:size(alpha,1),alpha*180/pi);hold on;
title('\fontsize{14} \fontname{Times New Roman} alpha trajectory');
xlabel('\fontsize{14} \fontname{Times New Roman} s/10');
ylabel('\fontsize{14} \fontname{Times New Roman} alpha(degree)');
axis([0,size(rho,1),-inf,inf]);
grid on;

subplot(2,3,4); plot(1:size(beta,1),beta*180/pi);hold on;
title('\fontsize{14} \fontname{Times New Roman} beta trajectory');
xlabel('\fontsize{14} \fontname{Times New Roman} s/10');
ylabel('\fontsize{14} \fontname{Times New Roman} beta(degree)');
axis([0,size(rho,1),-inf,inf]);
grid on;

subplot(2,3,5); plot(1:size(state,1),state);hold on;
title('\fontsize{14} \fontname{Times New Roman} state trajectory');
xlabel('\fontsize{14} \fontname{Times New Roman} s/10');
ylabel('\fontsize{14} \fontname{Times New Roman} state');
axis([0,size(vl,1),-inf,inf]);
grid on;

subplot(2,3,6); plot(1:size(vl,1),vl);hold on;
title('\fontsize{14} \fontname{Times New Roman} state trajectory');
xlabel('\fontsize{14} \fontname{Times New Roman} s/10');
ylabel('\fontsize{14} \fontname{Times New Roman} state');
axis([0,size(vl,1),-inf,inf]);
grid on;

subplot(2,3,6); plot(1:size(vr,1),vr);hold on;
title('\fontsize{14} \fontname{Times New Roman} vr trajectory');
xlabel('\fontsize{14} \fontname{Times New Roman} s/10');
ylabel('\fontsize{14} \fontname{Times New Roman} vr(speed)');
legend('vl','vr');
axis([0,size(vr,1),-inf,inf]);
grid on;



 

   