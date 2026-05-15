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

data_rCFD = importdata('./rCFD/rCFD_CoG.out',' ');

[n_rCFD, n_tmp] = size(data_rCFD)
%% rCFD post CoG

figure(1);

subplot(1,2,1);

size_vec = linspace(1,80,n_rCFD);

scatter(data_rCFD(:,2),data_rCFD(:,3),size_vec,...
    'MarkerFaceColor','r', 'MarkerEdgeColor','r')

grid on;
hold on;

title('rCFD Marker CoG','FontSize',1.2*FS,'Color','black','FontWeight','bold');
ylabel('y-coord (m)','Color','black','FontSize',FS);
xlabel('x-coord (m)','FontSize',FS);

axis equal;
axis([0 0.1 0 0.1])
ax = gca;
ax.FontSize = 14;
ax.YLabel.FontSize = 16;
ax.XLabel.FontSize = 16;
ax.Title.FontSize = 16;

subplot(1,2,2);

size_vec = linspace(1,80,n_rCFD);

scatter(data_rCFD(:,5),data_rCFD(:,6),size_vec,...
    'MarkerFaceColor','r','MarkerEdgeColor','r')

grid on;
hold on;

title('rCFD Marker CoG','FontSize',1.2*FS,'Color','black','FontWeight','bold');
ylabel('y-coord (m)','Color','black','FontSize',FS);
xlabel('x-coord (m)','FontSize',FS);

axis equal;
axis([0 0.1 0 0.1])
ax = gca;
ax.FontSize = 14;
ax.YLabel.FontSize = 16;
ax.XLabel.FontSize = 16;
ax.Title.FontSize = 16;

print(gcf, './rCFD/rCFD_CoG.jpg', '-djpeg');

%% Load data
clear data_rCFD;

data_rCFD = importdata('./rCFD/rCFD_mix_ind.out',' ');

[n_CFD, n_tmp] = size(data_rCFD)

%% rCFD post Mixing Index

figure(2);

plot(data_rCFD(:,1)-data_rCFD(1,1), data_rCFD(:,2),'r-','LineWidth', 2*LW)
grid on;
hold on;
plot(data_rCFD(:,1)-data_rCFD(1,1), data_rCFD(:,3),'r--','LineWidth', 2*LW)

title('rCFD Mixing Performance','FontSize',1.2*FS,'Color','black','FontWeight','bold');
ylabel('mixing index (.)','Color','black','FontSize',FS);
xlabel('Time (s)','FontSize',FS);

ax = gca;
ax.FontSize = 14;
ax.YLabel.FontSize = 16;
ax.XLabel.FontSize = 16;
ax.Title.FontSize = 16;

print(gcf, './rCFD/rCFD_mix_ind.jpg', '-djpeg');

