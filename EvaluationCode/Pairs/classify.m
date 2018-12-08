clc,clear;
addpath '..\libsvm-3.12\matlab\';
desc_fold = 'Desc\';

trainingDesc = [];
testingDesc = [];
trainingLbls = [];
testingLbls = [];
dDesc = dir([desc_fold '*.txt']);
trainActors = [6 7 8 9 10];
testActors = [1 2 3 4 5];
for i=1:length(dDesc)
    dname = dDesc(i).name;
    d = load([desc_fold dname]);
    ind = strfind(dname,'a');
    action = str2num(dname(ind(1)+1:ind(1)+2));
    ind = strfind(dname,'s');
    actor = str2num(dname(ind(1)+1:ind(1)+2));
    
    if isempty(find(testActors == actor)) % training;
        trainingDesc = [trainingDesc;d];
        trainingLbls = [trainingLbls;action];
    else                                  % testing
        testingDesc = [testingDesc;d];
        testingLbls = [testingLbls;action];
    end
end

% normalize descriptors
% scale every attribute to -1 to 1
trainingDesc = scaleDescs(trainingDesc);
testingDesc = scaleDescs(testingDesc);

svmParams = '-t 1 -g 0.125';
model = svmtrain(trainingLbls,trainingDesc,svmParams);
predicted_labels = svmpredict(testingLbls,testingDesc,model);
acc = (length(find((predicted_labels == testingLbls) == 1))/length(testingLbls))*100;



