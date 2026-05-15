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

data_CFD = importdata('./CFD_reference/CFD_mix_ind.out',' ');

[n_CFD, n_tmp] = size(data_CFD)
%% CFD post mix_ind

figure(1);

plot(data_CFD(:,1)-data_CFD(1,1), data_CFD(:,2),'k-','LineWidth', 2*LW)
grid on;
hold on;

title('CFD Mixing Performance','FontSize',1.2*FS,'Color','black','FontWeight','bold');
ylabel('mixing index (.)','Color','black','FontSize',FS);
xlabel('Time (s)','FontSize',FS);
ax = gca;
ax.FontSize = 14;
ax.YLabel.FontSize = 16;
ax.XLabel.FontSize = 16;
ax.Title.FontSize = 16;

print(gcf, './CFD_reference/CFD_mix_ind.jpg', '-djpeg');

%% Load data

data_CFD = importdata('./CFD_reference/CFD_mass.out',' ');

[n_CFD, n_tmp] = size(data_CFD)

%% CFD post mass

figure(2);

plot(data_CFD(:,1)-data_CFD(1,1), data_CFD(:,2),'k-','LineWidth', 2*LW)
grid on;
hold on;

title('CFD 2nd Gas Mass','FontSize',1.2*FS,'Color','black','FontWeight','bold');
ylabel('mass (kg)','Color','black','FontSize',FS);
xlabel('Time (s)','FontSize',FS);
ax = gca;
ax.FontSize = 14;
ax.YLabel.FontSize = 16;
ax.XLabel.FontSize = 16;
ax.Title.FontSize = 16;

print(gcf, './CFD_reference/CFD_mass.jpg', '-djpeg');

