<template>
  <div class="min-h-screen bg-gradient-to-b from-yellow-100 to-yellow-300 flex flex-col items-center relative">
    <!-- 背景大标题 -->
    <div class="absolute top-16 xs:top-24 left-1/2 -translate-x-1/2 select-none pointer-events-none z-0 w-full flex justify-center">
      <span class="text-3xl xs:text-5xl md:text-7xl font-extrabold text-yellow-400 drop-shadow-lg tracking-widest whitespace-nowrap">游戏大厅</span>
    </div>
    <!-- 顶部栏 -->
    <div class="w-full flex justify-between items-center px-4 md:px-8 py-6 z-10">
      <span class="text-lg md:text-2xl font-bold text-yellow-800">欢迎，{{ currentUser?.username || '游客' }}</span>
      <button class="btn-yellow" @click="handleLogout">退出登录</button>
    </div>
    <!-- 内容区 -->
    <div class="w-full max-w-6xl pt-24 grid grid-cols-1 md:grid-cols-2 gap-8 mt-6 z-10">
      <!-- 房间列表 -->
      <div class="bg-white/90 rounded-2xl shadow-lg p-6">
        <div class="flex justify-between items-center mb-4">
          <h2 class="text-2xl font-bold text-yellow-800">当前房间</h2>
          <div class="flex gap-2">
            <button class="btn-green" @click="showCreateRoom = true">创建房间</button>
            <button class="btn-yellow" @click="showBotDialog = true">人机对战</button>
          </div>
        </div>
        <div v-for="room in rooms" :key="room.id" class="mb-4 last:mb-0">
          <div class="flex items-center gap-4 p-4 bg-yellow-50 rounded-xl shadow-sm">
            <span class="font-bold text-lg">{{ room.name }}</span>
            <span class="text-gray-500">({{ room.player_count }}/2人)</span>
            <span v-if="room.has_password" class="ml-2 text-yellow-600 text-sm">🔒有密码</span>
            <span v-if="room.status === 'full'" class="text-blue-600 text-xs ml-2">已满，可观战</span>
            <button v-if="room.status !== 'full'" class="btn-blue ml-auto" @click="joinRoom(room)">
              加入
            </button>
            <button v-else class="btn-gray ml-auto" @click="watchRoom(room)">观战</button>
            
          </div>
        </div>
      </div>
      <!-- 在线用户列表 -->
      <div class="bg-white/90 rounded-2xl shadow-lg p-6">
        <h2 class="text-2xl font-bold text-yellow-800 mb-4">在线用户</h2>
        <div class="flex flex-wrap gap-3">
          <span v-for="user in onlineUsers" :key="user.username" class="px-3 py-1 rounded-lg bg-yellow-200 text-yellow-900 font-medium shadow-sm">
            {{ user.username }}
          </span>
        </div>
      </div>
    </div>
    <!-- 创建房间弹窗 -->
    <div v-if="showCreateRoom" class="fixed inset-0 z-50 flex items-center justify-center bg-black/40">
      <div class="bg-white rounded-2xl p-8 shadow-xl w-full max-w-sm flex flex-col gap-4">
        <h3 class="text-xl font-bold text-yellow-700 mb-2">创建房间</h3>
        <input v-model="roomName" type="text" placeholder="房间名" class="input" />
        <div class="flex items-center gap-2">
          <input v-model="usePassword" type="checkbox" id="usePassword" />
          <label for="usePassword" class="text-gray-700">设置房间密码</label>
        </div>
        <input v-if="usePassword" v-model="roomPassword" type="password" placeholder="密码" class="input" />
        <div class="flex gap-2 mt-4">
          <button class="btn-yellow flex-1" @click="createRoom">创建</button>
          <button class="btn-gray flex-1" @click="showCreateRoom = false">取消</button>
        </div>
        <p v-if="createError" class="text-red-600 text-sm">{{ createError }}</p>
      </div>
    </div>
    <!-- 人机对战难度选择弹窗 -->
    <div v-if="showBotDialog" class="fixed inset-0 z-50 flex items-center justify-center bg-black/40">
      <div class="bg-white rounded-2xl p-8 shadow-xl w-full max-w-sm flex flex-col gap-6">
        <h3 class="text-xl font-bold text-yellow-700 mb-2">选择人机对战难度</h3>
        <div class="flex flex-col gap-2">
          <label>
            <input type="radio" v-model="botLevel" value="easy" /> 简单
          </label>
          <label>
            <input type="radio" v-model="botLevel" value="normal" /> 中等
          </label>
          <label>
            <input type="radio" v-model="botLevel" value="hard" /> 困难
          </label>
        </div>
        <div class="flex gap-2 mt-4">
          <button class="btn-yellow flex-1" @click="createBotRoom">确定</button>
          <button class="btn-gray flex-1" @click="showBotDialog = false">取消</button>
        </div>
        <p v-if="botError" class="text-red-600 text-sm">{{ botError }}</p>
      </div>
    </div>
  </div>
</template>



<script setup>

import { ref, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import { get, post } from '@/utils/api'
import { sendWSMsg, closeWS } from '@/utils/ws'

const showBotDialog = ref(false)
const botLevel = ref('easy')
const botError = ref('')

const router = useRouter()

// 当前登录用户信息，建议登录成功后 localStorage.setItem('user', JSON.stringify({...}))
const currentUser = ref(JSON.parse(localStorage.getItem('user')) || null)

const rooms = ref([])
const onlineUsers = ref([])
const showCreateRoom = ref(false)
const roomName = ref('')
const usePassword = ref(false)
const roomPassword = ref('')
const createError = ref('')

// 拉取房间列表
async function fetchRooms() {
  const res = await get('/rooms') // 对应 GET /api/rooms
  // 后端返回 {success: true, list: [...]}
  rooms.value = res.list || []
}

// 拉取在线用户
async function fetchOnlineUsers() {
  const res = await get('/users/online') // 对应 GET /api/users/online
  // 后端返回 {success: true, users: [...]}
  onlineUsers.value = res.users || []
}

// 页面加载和每5秒自动刷新
onMounted(() => {
  fetchRooms()
  fetchOnlineUsers()
  setInterval(() => {
    fetchRooms()
    fetchOnlineUsers()
  }, 5000)
})

// 登出处理（后端可实现 /auth/logout）
async function handleLogout() {
  // 可以实现 /api/auth/logout，也可以本地清除
   await post('/auth/logout', { username: currentUser.value.username })
  sendWSMsg('logout', { user: currentUser.value.username })
  closeWS()
 // localStorage.removeItem('user')
  //localStorage.removeItem('token')
  router.push('/login')
}

// 创建房间
async function createRoom() {
  if (!roomName.value) {
    createError.value = '请填写房间名'
    return
  }
  const payload = {
    name: roomName.value,
    user_id: currentUser.value.id, // 必须int类型
    password: usePassword.value ? roomPassword.value : ''
  }
  const res = await post('/rooms', payload) // 对应 POST /api/rooms
  // 后端返回 {success:true, room_id:1}
  if (res.success && res.room_id) {
    router.push(`/room/${res.room_id}`)
  } else {
    createError.value = res.msg || '创建失败'
  }
}

async function createBotRoom() {
  const payload = {
    user_id: currentUser.value.id,
    ai: true,
    level: botLevel.value // easy/normal/hard
  }
  const res = await post('/rooms/bot', payload)
  if (res.success && res.room_id) {
    showBotDialog.value = false
    router.push(`/room/${res.room_id}`)
  } else {
    botError.value = res.msg || '创建AI房失败'
  }
}


// 加入房间
async function joinRoom(room) {
  let pwd = ''
  if (room.has_password) {
    pwd = prompt('请输入房间密码')
    if (pwd === null) return // 用户点取消
  }
  const res = await post('/rooms/join', {
    room_id: room.id, // 后端参数必须为 room_id
    user_id: currentUser.value.id,
    password: pwd
  })
  // 后端返回 {success:true, role:"black"/"white"/"spectator"}
  if (res.success) {
    router.push(`/room/${room.id}`)
  } else {
    alert(res.msg || '加入失败')
  }
}

// 观战（可选实现方式）
function watchRoom(room) {
  router.push(`/room/${room.id}?spectate=1`)
}


</script>


<style scoped>
.btn-yellow {
  @apply px-6 py-2 bg-yellow-400 hover:bg-yellow-500 text-white font-bold rounded-xl shadow transition;
}
.btn-green {
  @apply px-4 py-2 bg-green-400 hover:bg-green-500 text-white font-bold rounded-xl shadow transition;
}
.btn-blue {
  @apply px-4 py-2 bg-blue-400 hover:bg-blue-500 text-white font-bold rounded-xl shadow transition;
}
.btn-gray {
  @apply px-4 py-2 bg-gray-300 hover:bg-gray-400 text-gray-800 font-bold rounded-xl shadow transition;
}
.input {
  @apply p-3 rounded-lg border border-gray-300 focus:outline-none focus:ring-2 focus:ring-yellow-400 text-lg bg-white w-full;
}
</style>
