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

data_CFD = importdata('./CFD_start/CFD_startup_energy.out',' ');

[n_CFD, n_tmp] = size(data_CFD)
%% CFD top

figure(1);

n_CFD_start = floor(1*n_CFD/5);
n_CFD_start_mean = floor(4*n_CFD/5);

mean_resolved =   mean(data_CFD(n_CFD_start_mean:n_CFD,2))
mean_unresolved = mean(data_CFD(n_CFD_start_mean:n_CFD,3))

plot(data_CFD(n_CFD_start:n_CFD,1), data_CFD(n_CFD_start:n_CFD,2)/mean_resolved,...
    'k-','LineWidth', 2*LW)
grid on;
hold on;
plot(data_CFD(n_CFD_start:n_CFD,1), data_CFD(n_CFD_start:n_CFD,3)/mean_unresolved,...
    'k--','LineWidth', 2*LW)

title('Start-up Energy','FontSize',1.2*FS,'Color','black','FontWeight','bold');
ylabel('kinetic energy (m2/s2)','Color','black','FontSize',FS);
xlabel('Time (s)','FontSize',FS);
legend({'resolved','unresolved'},'FontSize',14, 'Location','northeast');
ax = gca;
ax.FontSize = 14;
ax.YLabel.FontSize = 16;
ax.XLabel.FontSize = 16;
ax.Title.FontSize = 16;

print(gcf, './CFD_start/CFD_start_energy.jpg', '-djpeg');

