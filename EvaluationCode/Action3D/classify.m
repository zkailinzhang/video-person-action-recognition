% write the classification function
clc,clear,close all;
addpath '..\libsvm-3.12\matlab\';
desc_fold = 'Desc\';

trainingDesc = [];
testingDesc = [];
trainingLbls = [];
testingLbls = [];
dDesc = dir([desc_fold '*.txt']);
trainActors = [1 2 3 4 5];
testActors = [6 7 8 9 10];
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
trainingDesc = scaleDescs(trainingDesc);
testingDesc = scaleDescs(testingDesc);

svmParams = '-t 1 -g 0.125 -d 3';
model = svmtrain(trainingLbls,trainingDesc,svmParams);
predicted_labels = svmpredict(testingLbls,testingDesc,model);
acc = (length(find((predicted_labels == testingLbls) == 1))/length(testingLbls))*100;

