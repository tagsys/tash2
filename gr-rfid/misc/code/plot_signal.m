clc
clear all
close all

fi_1 = fopen('../data/source','rb');
x_inter_1 = fread(fi_1, 'float32');

% if data is complex
x_1 = x_inter_1(1:2:end) + 1i*x_inter_1(2:2:end);

%plot(abs(x_1(1:20e4)))
data = [abs(x_1(2.65e5:2.84672e5));abs(x_1(2.6e5:2.65e5))];
plot(data);
xlabel('samples');
ylabel('amplitude');
grid on;