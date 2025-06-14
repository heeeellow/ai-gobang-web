<template>
  <div class="min-h-screen flex flex-col items-center bg-gradient-to-b from-yellow-100 to-yellow-300 relative">
    <!-- 顶部玩家信息 -->
    <div class="z-10 w-full max-w-3xl flex flex-row justify-between items-center py-6 px-2 md:px-8">
      <div>
        <span class="font-bold text-xl text-yellow-800">黑方：</span>
        <span class="font-bold text-lg text-gray-700">{{ blackUser }}</span>
      </div>
      <div>
        <span class="font-bold text-xl text-yellow-800">白方：</span>
        <span class="font-bold text-lg text-gray-700">{{ whiteUser }}</span>
      </div>
      <div v-if="isSpectator" class="text-blue-600 font-bold">观战中</div>
    </div>
    <!-- 棋盘与侧栏 -->
    <div class="z-10 pt-24 w-full max-w-5xl flex flex-col md:flex-row gap-8 items-start justify-center">
      <!-- 聊天区 -->
      <div class="flex flex-col bg-white/80 rounded-xl shadow p-2 mt-4 h-72 w-full md:w-[260px]">
        <div class="font-bold mb-1 text-yellow-700">房间聊天</div>
        <div ref="chatListRef" class="flex-1 overflow-y-auto space-y-1 p-1 border rounded bg-white/60">
          <div v-for="(msg, idx) in chatMessages" :key="idx" class="text-sm">
            <span class="font-semibold text-gray-700">{{ msg.user }}：</span>
            <span class="text-gray-600">{{ msg.text }}</span>
            <span class="float-right text-xs text-gray-400">{{ msg.time }}</span>
          </div>
        </div>
        <div class="flex gap-1 mt-1">
          <input v-model="chatInput"
                 @keydown.enter="sendChat"
                 :disabled="isSpectator"
                 placeholder="输入消息并回车"
                 class="flex-1 p-2 rounded border focus:outline-none focus:ring-2 focus:ring-yellow-400 text-sm"/>
          <button @click="sendChat" class="btn-yellow px-3 py-1 text-sm">发送</button>
        </div>
      </div>
      <!-- 棋盘区域 -->
      <div class="flex flex-col items-center">
        <!-- 当前回合和倒计时 -->
        <div class="mb-2 flex gap-4 items-center">
          <span v-if="gameStarted && !gameOver" class="font-bold text-lg text-gray-700">
            当前回合：<span :class="currentColor === 'black' ? 'text-black' : 'text-gray-500'">{{ currentTurnUser }}</span>
          </span>
          <span v-if="gameStarted && !gameOver" class="text-red-600 font-bold text-lg">倒计时：{{ timer }}</span>
        </div>
        <!-- 棋盘 -->
        <div class="relative aspect-square w-[430px] max-w-[90vw] bg-yellow-200 rounded-xl shadow-inner border-4 border-yellow-400">
          <!-- 棋盘网格 -->
          <svg :width="boardPx" :height="boardPx" class="absolute left-0 top-0">
            <g>
              <line
                v-for="i in 15"
                :key="'h'+i"
                :x1="cellSize/2"
                :y1="cellSize/2 + (i-1)*cellSize"
                :x2="cellSize/2 + 14*cellSize"
                :y2="cellSize/2 + (i-1)*cellSize"
                stroke="#bfa657"
                stroke-width="2"
              />
              <line
                v-for="i in 15"
                :key="'v'+i"
                :x1="cellSize/2 + (i-1)*cellSize"
                :y1="cellSize/2"
                :x2="cellSize/2 + (i-1)*cellSize"
                :y2="cellSize/2 + 14*cellSize"
                stroke="#bfa657"
                stroke-width="2"
              />
              <!-- 星位 -->
              <circle
                v-for="(pt, idx) in starPoints"
                :key="'star'+idx"
                :cx="cellSize/2 + pt[0]*cellSize"
                :cy="cellSize/2 + pt[1]*cellSize"
                r="5"
                fill="#9d7716"
              />
            </g>
          </svg>
          <!-- 棋子 -->
          <transition-group name="drop" tag="div">
            <div
              v-for="(chess, idx) in board"
              :key="chess.x + '-' + chess.y"
              :style="getStoneStyle(chess.x, chess.y)"
              class="absolute z-10"
            >
              <div
                :class="chess.color === 'black' ? 'bg-gradient-to-b from-gray-800 to-black' : 'bg-gradient-to-t from-gray-300 to-white'"
                class="rounded-full shadow-lg border-2 border-black w-7 h-7 flex items-center justify-center drop-shadow transition-all"
                :style="{ transform: stoneDropAnim === idx ? 'scale(1.25)' : 'scale(1)' }"
              ></div>
            </div>
          </transition-group>
          <!-- 点击落子交互 -->
          <div
            v-if="gameStarted && !gameOver && !isSpectator"
            class="absolute inset-0"
            @click="handleBoardClick($event)"
          ></div>
        </div>
        <!-- 按钮区 -->
        <div class="flex gap-4 mt-6">
          <button
            v-if="!gameStarted && !isSpectator && !ready"
            class="btn-yellow"
            @click="handleReady"
          >准备</button>
          <span v-else-if="!gameStarted && !isSpectator" class="text-green-700 font-bold">已准备，等待对手...</span>
          <button
            v-if="gameStarted && !gameOver && !isSpectator"
            class="btn-red"
            @click="handleGiveUp"
          >认输</button>
          <button
            v-if="!gameStarted || gameOver"
            class="btn-gray"
            @click="handleReturn"
          >返回大厅</button>
        </div>
      </div>
      <!-- 右侧状态栏 -->
      <div class="flex flex-col gap-2 min-w-[180px] w-full md:w-[200px]">
        <div class="p-4 bg-white/90 rounded-xl shadow text-gray-600 mb-4">
          <div v-if="isSpectator" class="font-bold text-blue-600">您是观战者，不能落子。</div>
          <div v-else>
            <div v-if="!gameStarted">准备后游戏自动开始。</div>
            <div v-else-if="gameStarted && !gameOver">对局进行中...</div>
            <div v-else>对局结束</div>
          </div>
        </div>
      </div>
    </div>
    <!-- 结束弹窗 -->
    <div v-if="gameOver" class="fixed inset-0 flex items-center justify-center z-50 bg-black/30">
      <div class="bg-white rounded-2xl px-12 py-10 text-center shadow-2xl flex flex-col gap-6">
        <h2 class="text-2xl font-bold text-yellow-700">{{ resultMsg }}</h2>
        <button class="btn-yellow" @click="handleReturn">返回大厅</button>
      </div>
    </div>
  </div>
</template>


<script setup>
import { nextTick, ref, computed, onMounted, onUnmounted } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import { connectWS, onWSMsg, sendWSMsg, closeWS } from '@/utils/ws'
import { post } from '@/utils/api'
const route = useRoute()
const router = useRouter()
const user = JSON.parse(localStorage.getItem('user')) || { username: '游客', id: 0 }
const token = localStorage.getItem('token') || ''
const roomId = Number(route.params.id)
const isSpectator = computed(() => !!route.query.spectate)

const blackUser = ref('')
const whiteUser = ref('')
const currentTurnUser = ref('')
const board = ref([])
const currentColor = ref('black')
const gameStarted = ref(false)
const gameOver = ref(false)
const ready = ref(false)
const resultMsg = ref('')
const timer = ref(60)
let timerInterval = null

const chessMatrix = ref(Array.from({ length: 15 }, () => Array(15).fill(null)))
const boardPx = 420
const cellSize = boardPx / 15
const starPoints = [
  [3, 3], [3, 7], [3, 11],
  [7, 3], [7, 7], [7, 11],
  [11, 3], [11, 7], [11, 11],
]
const stoneDropAnim = ref(-1)

const chatMessages = ref([])
const chatInput = ref('')
const chatListRef = ref(null)

onMounted(() => {
connectWS(token)
  sendWSMsg('join_room', { 
  roomId, 
  user_id: user.id,        // 必须带上
  username: user.username, // 必须带上
  spectate: isSpectator.value 
})
  onWSMsg(handleWSMsg)
})


onUnmounted(() => {
  closeWS()
  clearInterval(timerInterval)
})

function handleWSMsg(msg) {
  if (msg.type === 'room_info') {
    blackUser.value = msg.black
    whiteUser.value = msg.white
    currentTurnUser.value = msg.currentUser
    currentColor.value = msg.currentColor
    board.value = msg.board || []
    chessMatrix.value = Array.from({ length: 15 }, () => Array(15).fill(null))
    for (const c of (msg.board || [])) {
      chessMatrix.value[c.y][c.x] = c.color
    }
    gameStarted.value = !!msg.started
    gameOver.value = !!msg.over
    resultMsg.value = msg.resultMsg || ''
  }
  if (msg.type === 'chess_move') {
    board.value.push({ x: msg.x, y: msg.y, color: msg.color })
    chessMatrix.value[msg.y][msg.x] = msg.color
    currentColor.value = msg.nextColor
    currentTurnUser.value = msg.nextUser
    stoneDropAnim.value = board.value.length - 1
    setTimeout(() => { stoneDropAnim.value = -1 }, 180)
    timer.value = 60
    resetTimer()
  }
  if (msg.type === 'chat') {
    chatMessages.value.push({
      user: msg.user,
      text: msg.text,
      time: msg.time,
    })
    nextTick(() => {
      if (chatListRef.value) chatListRef.value.scrollTop = chatListRef.value.scrollHeight
    })
  }
  if (msg.type === 'ready') {
    gameStarted.value = !!msg.started
  }
  if (msg.type === 'game_over') {
    gameOver.value = true
    resultMsg.value = msg.resultMsg
    clearInterval(timerInterval)
  }
}

function sendChat() {
  if (!chatInput.value.trim()) return
  sendWSMsg('chat', { roomId, text: chatInput.value })
  chatInput.value = ''
}

function handleBoardClick(e) {
  if (!gameStarted.value || gameOver.value || isSpectator.value) return
  const rect = e.currentTarget.getBoundingClientRect()
  const x = Math.round((e.clientX - rect.left - cellSize/2) / cellSize)
  const y = Math.round((e.clientY - rect.top - cellSize/2) / cellSize)
  if (x < 0 || x > 14 || y < 0 || y > 14) return
  if (chessMatrix.value[y][x]) return
  if (user.username !== currentTurnUser.value) return
  sendWSMsg('chess_move', { roomId, x, y })
}

function getStoneStyle(x, y) {
  return {
    left: `${cellSize/2 + x*cellSize - cellSize/2}px`,
    top: `${cellSize/2 + y*cellSize - cellSize/2}px`,
    transition: 'transform 0.18s',
  }
}

function resetTimer() {
  timer.value = 60
  clearInterval(timerInterval)
  timerInterval = setInterval(() => {
    timer.value--
    if (timer.value <= 0) {
      clearInterval(timerInterval)
      sendWSMsg('timeout', { roomId })
    }
  }, 1000)
}

function handleReady() {
  ready.value = true
  sendWSMsg('ready', { roomId })
}
function handleGiveUp() {
  sendWSMsg('giveup', { roomId })
}
async function handleReturn() {
   await post('/rooms/leave', {          // 通知后端删除数据库房间成员
    room_id: roomId,
    user_id: user.id,
  })
  sendWSMsg('leave_room', { roomId })
  router.push('/lobby')
}
</script>


<style scoped>
.btn-yellow {
  @apply px-6 py-2 bg-yellow-400 hover:bg-yellow-500 text-white font-bold rounded-xl shadow transition;
}
.btn-red {
  @apply px-6 py-2 bg-red-400 hover:bg-red-500 text-white font-bold rounded-xl shadow transition;
}
.btn-gray {
  @apply px-6 py-2 bg-gray-300 hover:bg-gray-400 text-gray-800 font-bold rounded-xl shadow transition;
}
.drop-enter-from {
  transform: scale(0.5) translateY(-20px);
  opacity: 0;
}
.drop-enter-active {
  transition: all 0.18s cubic-bezier(0.35,1.2,0.7,1.2);
}
.drop-enter-to {
  transform: scale(1) translateY(0);
  opacity: 1;
}
</style>
