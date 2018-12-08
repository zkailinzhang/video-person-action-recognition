clc,clear,close all;
addpath '..\libsvm-3.12\matlab\';

desc_fold = 'Desc\';
trainingDesc = [];
testingDesc = [];
trainingLbls = [];
testingLbls = [];
dDesc = dir([desc_fold '*_features_HONV.mat']);

trainActors = [1 3 5 7 9];
testActors = [2 4 6 8 10];
for i=1:length(dDesc)
    dname = dDesc(i).name;
    d = load([desc_fold dname]);
    ind = strfind(dname,'a');
    action = str2num(dname(ind(1)+1:ind(1)+2));
    ind = strfind(dname,'s');
    actor = str2num(dname(ind(1)+1:ind(1)+2));
    
    if isempty(find(testActors == actor)) % training;
        trainingDesc = [trainingDesc;d.fea(:)'];
        trainingLbls = [trainingLbls;action];
    else                                  % testing
        testingDesc = [testingDesc;d.fea(:)'];
        testingLbls = [testingLbls;action];
    end
end

% normalize descriptors
% scale every attribute to -1 to 1
trainingDesc = scaleDescs(trainingDesc);
testingDesc = scaleDescs(testingDesc);

svmParams = '-t 0 -b 1';
model = svmtrain(trainingLbls,trainingDesc,svmParams);
[predicted_labels,accuracy, prob_estimates] = svmpredict(testingLbls,testingDesc,model,['-b 1']);

