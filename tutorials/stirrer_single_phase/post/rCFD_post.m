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
%% CFD top

figure(1);

plot(data_rCFD(:,1)-data_rCFD(1,1), data_rCFD(:,2),'r-','LineWidth', 2*LW)
grid on;
hold on;
plot(data_rCFD(:,1)-data_rCFD(1,1), data_rCFD(:,3),'r--','LineWidth', 2*LW)

title('Mixing Performance','FontSize',1.2*FS,'Color','black','FontWeight','bold');
ylabel('mixing index (.)','Color','black','FontSize',FS);
xlabel('Time (s)','FontSize',FS);
legend({'vertical','radial'},'FontSize',14, 'Location','northwest');
ax = gca;
ax.FontSize = 14;
ax.YLabel.FontSize = 16;
ax.XLabel.FontSize = 16;
ax.Title.FontSize = 16;

print(gcf, './rCFD/rCFD_mix_ind.jpg', '-djpeg');

