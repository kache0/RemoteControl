# 導入需要的模組
import discord
import requests
from discord.ext import commands, tasks
from discord import app_commands
import subprocess


# 定義全局變數保存頻道 ID
ping_channel_id = None

# 初始化機器人和權限
intents = discord.Intents.default()
intents.message_content = True
bot = commands.Bot(command_prefix="!", intents=intents)

TOKEN = ''

# 機器人啟動事件
@bot.event
async def on_ready():
    print(f"目前登入身份 --> {bot.user}")
    try:
        # 同步應用命令
        synced = await bot.tree.sync()
        print(f"已同步 {len(synced)} 個應用命令。")
    except Exception as e:
        print(f"同步應用命令失敗: {e}")

# 應用命令：顯示按鈕
@bot.tree.command(name="button_interaction_on", description="啟用按鈕交互")
async def button_interaction_on(interaction: discord.Interaction):
    
    global ping_channel_id
    # 記錄頻道 ID
    ping_channel_id = interaction.channel.id
    print(f"已記錄頻道 ID: {ping_channel_id}")
    
    # 創建 View 和按鈕
    view = discord.ui.View()
    button1 = discord.ui.Button(
        label="開機/關機",
        style=discord.ButtonStyle.success,
        custom_id="PowerOn/Off"
    )
    button2 = discord.ui.Button(
        label="強制關機",
        style=discord.ButtonStyle.danger,
        custom_id="PowerForceOff"
    )
    button3 = discord.ui.Button(
        label="重啟",
        style=discord.ButtonStyle.primary,
        custom_id="PowerReboot"
    )
    view.add_item(button1)
    view.add_item(button2)
    view.add_item(button3)
   
    if not ping_check.is_running():
        ping_check.start()
    # 發送消息，附帶按鈕
    await interaction.response.send_message(view=view) 
    print("按鈕已成功發送，等待用戶交互。") 
    

# 每30秒檢查一次ping結果
@tasks.loop(seconds=30)
async def ping_check():
    global ping_channel_id
    try:
        # 獲取目標頻道
        channel = bot.get_channel(ping_channel_id)
        if not channel:
            print("無法找到目標頻道")
            return

        # 獲取頻道中的最後一條訊息
        async for msg in channel.history(limit=1):
            if msg.author == bot.user:  # 確保是機器人發送的訊息
                # 執行 ping 命令
                result = subprocess.run(['ping', '192.168.232.234', '-n', '1'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
                print(result)
                ping_status = "🟢電腦已開機" if result.returncode == 0 else "🔴電腦已關機"

                # 更新訊息內容
                await msg.edit(content=f"電腦狀態: {ping_status}")
                break
        else:
            # 如果頻道中沒有機器人發送的訊息，則發送一條新訊息
            await channel.send("正在檢測伺服器狀態...")

    except Exception as e:
        print(f"檢查過程中出現錯誤: {e}")

    
# 監聽按鈕交互事件
@bot.event
async def on_interaction(interaction: discord.Interaction):
    if interaction.data and "custom_id" in interaction.data:
        custom_id = interaction.data["custom_id"]
        url = "http://192.168.232.234/powerSW"  
        data = {"password": "27835"}
        if custom_id == "PowerOn/Off":
            data["state"] = "PowerOn/Off"
            await interaction.response.edit_message(content="開機/關機成功")
        elif custom_id == "PowerForceOff":
            data["state"] = "PowerForceOff"
            await interaction.response.edit_message(content="強制關機成功")
        elif custom_id == "PowerReboot":
            data["state"] = "PowerReboot"
            await interaction.response.edit_message(content="重啟成功")
        elif custom_id == "SetTime":
            data["state"] = "SetTime"
            await interaction.response.edit_message(content="時間設定成功")
        else:
            print(f"未處理的 custom_id: {custom_id}")
            return
        try:
            # 發送 POST 請求
            requests.post(url, json=data)
            #await interaction.response.send_message("請求成功！")
        except Exception as e:
            print(e)
            await interaction.response.send_message("請求失敗！")

# 啟動機器人
bot.run(TOKEN)
