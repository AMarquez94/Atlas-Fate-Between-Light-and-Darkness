
print('This is lua')
SLB.using(SLB)

print(LogicManager)

lm = LogicManager();

val=lm:GetPlayerLife();

print(val);

lm:RespawnPlayer();

val=lm:GetPlayerLife();

print(val);

lm:TeleportPlayer(1,2,3);

print('Numero de agentes');
print(lm.numagents);



function OnPlayerKilled()
print("Player comes back to life");
lm:RespawnPlayer();
val=lm:GetPlayerLife();
print(val);
lm:TeleportPlayer(100,100,100);
end
