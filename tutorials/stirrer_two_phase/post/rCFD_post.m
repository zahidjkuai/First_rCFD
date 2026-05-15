% 2026
% Stefan Pirker
% Department of particulate flow modelling
% Linz, Austria

%% clear all

clear all;
close all;
clc;

%% post-proc param's

FS = 16;
MS = 10;
LW = 1;

%% Load data

data_rCFD = importdata('./rCFD/rCFD_Oxy.out',' ');

[n_CFD, n_tmp] = size(data_rCFD)
%% CFD top

figure(1);

Oxy_init = 8e-6;
total_mass = data_rCFD(1,2)/Oxy_init;

plot(data_rCFD(:,1)-data_rCFD(1,1), data_rCFD(:,2)/total_mass*1e6,'r-','LineWidth', 2*LW)
grid on;
hold on;

title('rCFD Oxygen Dissolution','FontSize',1.2*FS,'Color','black','FontWeight','bold');
ylabel('oxygen conc (ppm)','Color','black','FontSize',FS);
xlabel('Time (s)','FontSize',FS);

ax = gca;
ax.FontSize = 14;
ax.YLabel.FontSize = 16;
ax.XLabel.FontSize = 16;
ax.Title.FontSize = 16;

print(gcf, './rCFD/rCFD_Oxy.jpg', '-djpeg');

