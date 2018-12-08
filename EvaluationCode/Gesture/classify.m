clc,clear,close all;
addpath '..\libsvm-3.12\matlab\';
desc_fold = 'Desc\';

actors = [1:10];
dDesc = dir([desc_fold '*.txt']);

% read all descriptors
allDescs = [];
allLabels = [];
for i=1:length(dDesc)
    dname = dDesc(i).name;
    d = load([desc_fold dname]);
    allDescs = [allDescs;d];
end
allDescs = scaleDescs(allDescs);

% leave on subject out
all_accs = [];
all_nums = [];
for indLoop=1:10
    testActors = actors(indLoop);
    trainActors = setdiff(actors,testActors);
    trainingInd = [];
    testingInd = [];
    for i=1:length(dDesc)
        dname = dDesc(i).name;
        d = allDescs(i,:);
        ind = strfind(dname,'a');
        action = str2num(dname(ind(1)+1:ind(1)+2));
        ind = strfind(dname,'s');
        actor = str2num(dname(ind(1)+1:ind(1)+2));
        
        if ~isempty(find(trainActors == actor)) % training;
            trainingInd = [trainingInd;i];
        elseif ~isempty(find(testActors == actor)) % testing
            testingInd = [testingInd;i];
        end
        allLabels = [allLabels;action];
    end

    trainingDesc = allDescs(trainingInd,:);
    trainingLbls = allLabels(trainingInd);
    testingDesc = allDescs(testingInd,:);
    testingLbls = allLabels(testingInd);

    svmParams = '-t 1 -g 0.125';
    model = svmtrain(trainingLbls,trainingDesc,svmParams);
    predicted_labels = svmpredict(testingLbls,testingDesc,model);
    acc = (length(find((predicted_labels == testingLbls) == 1))/length(testingLbls))*100;
    all_accs = [all_accs,acc];
    all_nums = [all_nums,length(predicted_labels)];
end
acc = mean(all_accs)



