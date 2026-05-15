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

data_CFD = importdata('./CFD_reference/CFD_CoG.out',' ');

[n_CFD, n_tmp] = size(data_CFD)
%% CFD post CoG

figure(1);

subplot(1,2,1);

size_vec = linspace(1,80,n_CFD);

scatter(data_CFD(:,2),data_CFD(:,3),size_vec,...
    'MarkerFaceColor','k', 'MarkerEdgeColor','k')

grid on;
hold on;

title('CFD CoG, Diff small','FontSize',1.2*FS,'Color','black','FontWeight','bold');
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

scatter(data_CFD(:,5), data_CFD(:,6),size_vec,...
    'MarkerFaceColor','k','MarkerEdgeColor','k')

grid on;
hold on;

title('CFD CoG, Diff large','FontSize',1.2*FS,'Color','black','FontWeight','bold');
ylabel('y-coord (m)','Color','black','FontSize',FS);
xlabel('x-coord (m)','FontSize',FS);

axis equal;
axis([0 0.1 0 0.1])
ax = gca;
ax.FontSize = 14;
ax.YLabel.FontSize = 16;
ax.XLabel.FontSize = 16;
ax.Title.FontSize = 16;

print(gcf, './CFD_reference/CFD_CoG.jpg', '-djpeg');

%% Load data
clear data_CFD;

data_CFD = importdata('./CFD_reference/CFD_mix_ind.out',' ');

[n_CFD, n_tmp] = size(data_CFD)

%% CFD post Mixing Index

figure(2);

plot(data_CFD(:,1)-data_CFD(1,1), data_CFD(:,2),'k-','LineWidth', 2*LW)
grid on;
hold on;
plot(data_CFD(:,1)-data_CFD(1,1), data_CFD(:,3),'k--','LineWidth', 2*LW)

title('CFD Mixing Performance','FontSize',1.2*FS,'Color','black','FontWeight','bold');
ylabel('mixing index (.)','Color','black','FontSize',FS);
xlabel('Time (s)','FontSize',FS);
legend({'D = 1e-5','D = 3e-2'},'FontSize',14, 'Location','northwest');
ax = gca;
ax.FontSize = 14;
ax.YLabel.FontSize = 16;
ax.XLabel.FontSize = 16;
ax.Title.FontSize = 16;

print(gcf, './CFD_reference/CFD_mix_ind.jpg', '-djpeg');

