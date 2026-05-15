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

data_rCFD = importdata('./rCFD/rCFD_mix_ind.out',' ');

[n_CFD, n_tmp] = size(data_rCFD)
%% rCFD post mix_ind

figure(1);

plot(data_rCFD(:,1)-data_rCFD(1,1), data_rCFD(:,2),'r-','LineWidth', 2*LW)
grid on;
hold on;

title('rCFD Mixing Performance','FontSize',1.2*FS,'Color','black','FontWeight','bold');
ylabel('mixing index (.)','Color','black','FontSize',FS);
xlabel('Time (s)','FontSize',FS);
ax = gca;
ax.FontSize = 14;
ax.YLabel.FontSize = 16;
ax.XLabel.FontSize = 16;
ax.Title.FontSize = 16;

print(gcf, './rCFD/rCFD_mix_ind.jpg', '-djpeg');

%% Load data

data_rCFD = importdata('./rCFD/rCFD_mass.out',' ');

[n_CFD, n_tmp] = size(data_rCFD)

%% CFD post mass

figure(2);

plot(data_rCFD(:,1)-data_rCFD(1,1), data_rCFD(:,2),'r-','LineWidth', 2*LW)
grid on;
hold on;

title('rCFD 2nd Gas Mass','FontSize',1.2*FS,'Color','black','FontWeight','bold');
ylabel('mass (kg)','Color','black','FontSize',FS);
xlabel('Time (s)','FontSize',FS);
ax = gca;
ax.FontSize = 14;
ax.YLabel.FontSize = 16;
ax.XLabel.FontSize = 16;
ax.Title.FontSize = 16;

print(gcf, './rCFD/rCFD_mass.jpg', '-djpeg');

