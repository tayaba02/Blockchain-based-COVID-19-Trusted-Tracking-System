pragma solidity =0.6.6;

contract Registration{
    address public owner;
   
    struct oracle_type{
        bool exists;
        int infected;
        int recovered;
        int dead;
        int reputation;
        uint lastUpdate;
   }
    
    mapping(address=>oracle_type) public Oracles;
    
    address[] oraclesArray;
    
    int public averageinfected;
    int public averagerecovered;
    int public averagedead; 
    
    
    event OracleRegistered(address oracleAddress);

        modifier onlyOwner() { 
        require(
            msg.sender == owner,
            "Only contract owner can call this."
        );
        _;
    }
   
   constructor() public{
       owner=msg.sender;
   }
    function registerOracle (address oracleAddress) public onlyOwner{
       require(
            !Oracles[oracleAddress].exists,
            "Oracle already registred."
        );
       Oracles[oracleAddress].exists=true;
       Oracles[oracleAddress].reputation=80;
       oraclesArray.push(oracleAddress);
       emit OracleRegistered(oracleAddress);
   }

}

contract Reputation is Registration{
    
    int public trustworthiness_threshold;
    
   event ReputationUpdated(address oracleAddress, int reputationScore);

    constructor() public{
        trustworthiness_threshold = 90;

    }
    function computeReputation(address oracleAddress) public{
        
        int currentInfected = Oracles[oracleAddress].infected;
        int cr;
        int trustworthiness = 0;
        int adjusting_factor = 4;
        trustworthiness = averageinfected - currentInfected;
        if(trustworthiness<0){
            trustworthiness *= -1;
        }
        trustworthiness = 100 - trustworthiness;

        if(trustworthiness>trustworthiness_threshold){
            cr = (Oracles[oracleAddress].reputation*trustworthiness)/(4*adjusting_factor);
            cr /= 100;
            Oracles[oracleAddress].reputation += cr;
        }
        else{
            cr = (Oracles[oracleAddress].reputation*trustworthiness)/(4*(10-adjusting_factor));
            cr /= 100;
            Oracles[oracleAddress].reputation -= cr;
        }
        if (Oracles[oracleAddress].reputation<0){
            Oracles[oracleAddress].reputation = 0;
        }
        else if (Oracles[oracleAddress].reputation>100){
            Oracles[oracleAddress].reputation = 100;
        }
        
        emit ReputationUpdated(oracleAddress, Oracles[oracleAddress].reputation);
    }
    
}

contract Aggregator is Reputation{
    

    int tolerance_thresold;

    struct head{
       int value;
       int votes;
       int infectedCenter;
       int reocveredCenter;
       int deadCenter;
       int cr;
   }
   
   head[] public heads;
   
    constructor()public{
        owner = msg.sender;
        averageinfected= 0;
        averagerecovered= 0;
        averagedead=0; 
        tolerance_thresold = 5;

    }
   

    modifier onlyOracle{ 
        require(
            Oracles[msg.sender].exists,
            "Only Oracle can call this."
        );
        _;
    }
    
   event OracleInput(address oracleAddress, int newInfected,int newRecovered, int newDead);
   
   event LatestStatistics(int Infected, int Recovered, int Dead);

   function inputOracle(int infect,int recover,int  death) public onlyOracle {
       
       Oracles[msg.sender].infected+=infect;
       Oracles[msg.sender].recovered+=recover;
       Oracles[msg.sender].dead+=death;
       
       Oracles[msg.sender].lastUpdate=now;

       emit OracleInput(msg.sender, Oracles[msg.sender].infected, Oracles[msg.sender].recovered, Oracles[msg.sender].dead);


   }
    
    function calculatestatistics() public{
        
        head memory t;
        delete heads;
        t.votes = 1;
        t.value = Oracles[oraclesArray[0]].infected;
        t.infectedCenter = t.value;
        t.reocveredCenter = Oracles[oraclesArray[0]].recovered;
        t.deadCenter = Oracles[oraclesArray[0]].dead;
        t.cr = Oracles[oraclesArray[0]].reputation;
        heads.push(t);
        for(uint i = 1;i<oraclesArray.length;i++){
            int min = 1000;
            uint min_index;
                    for(uint j = 0;j<heads.length;j++){
                        int temp = heads[j].value-Oracles[oraclesArray[i]].infected;
                        if(temp<=tolerance_thresold && temp<min && temp>=0){
                            min = temp;
                            min_index = j;
                        }
                        else {
                            temp *= -1;
                            if(temp<=tolerance_thresold && temp<min && temp>=0){
                                min = temp;
                                min_index = j;
                            }
                        }
                    }
                    if(min!=1000){
                        heads[min_index].votes++;
                        heads[min_index].cr += Oracles[oraclesArray[i]].reputation;
                        heads[min_index].infectedCenter += Oracles[oraclesArray[i]].infected;
                        heads[min_index].reocveredCenter += Oracles[oraclesArray[i]].recovered;
                        heads[min_index].deadCenter += Oracles[oraclesArray[i]].dead;
                    }
                    else{
                        t.votes = 1;
                        t.value = Oracles[oraclesArray[i]].infected;
                        t.infectedCenter = t.value;
                        t.reocveredCenter = Oracles[oraclesArray[i]].infected;
                        t.deadCenter = Oracles[oraclesArray[i]].recovered;
                        t.cr = Oracles[oraclesArray[i]].dead;
                        heads.push(t);
                    }
        }
        int max = 0;
        for(uint j = 0;j<heads.length;j++){
                     if(heads[j].cr>=max){
                         max = heads[j].cr;
                         averageinfected = heads[j].infectedCenter/heads[j].votes;
                         averagerecovered = heads[j].reocveredCenter/heads[j].votes;
                         averagedead = heads[j].deadCenter/heads[j].votes;
                     }
        }
        
        emit LatestStatistics(averageinfected,averagerecovered,averagedead);
    }
    
}
  

